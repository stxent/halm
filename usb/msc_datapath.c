/*
 * msc_datapath.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/usb/msc.h>
#include <halm/usb/msc_datapath.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_trace.h>
#include <inttypes.h>
/*----------------------------------------------------------------------------*/
static bool enqueueUsbRx(struct MscQueryHandler *, uintptr_t, size_t,
    UsbRequestCallback, UsbRequestCallback, size_t *);
static bool enqueueUsbRxRequests(struct MscQueryHandler *, struct MscQuery *);
static bool enqueueUsbTx(struct MscQueryHandler *, uintptr_t, size_t,
    UsbRequestCallback, UsbRequestCallback, size_t *);
static bool enqueueUsbTxRequests(struct MscQueryHandler *, struct MscQuery *);
static void fillStorageReadQueue(struct MscQueryHandler *);
static void fillUsbReadQueue(struct MscQueryHandler *);
static void handleIncomingFlow(struct MscQueryHandler *);
static void handleOutgoingFlow(struct MscQueryHandler *);
static void makePingPongTransfer(struct MscQueryHandler *, void *, size_t,
    uint64_t, size_t);
static size_t prepareDataRx(struct MscQueryHandler *, struct UsbRequest *,
    uintptr_t, size_t, UsbRequestCallback, UsbRequestCallback);
static size_t prepareDataTx(struct MscQueryHandler *, struct UsbRequest *,
    uintptr_t, size_t, UsbRequestCallback, UsbRequestCallback);
static void resetTransferPool(struct MscQueryHandler *);
static bool storageRead(struct MscQueryHandler *, struct MscQuery *);
static void storageReadCallback(void *);
static bool storageWrite(struct MscQueryHandler *, struct MscQuery *);
static void storageWriteCallback(void *);
static void usbControlCallback(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void usbRxLastCallback(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void usbRxSilentCallback(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void usbTxLastCallback(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void usbTxSilentCallback(void *, struct UsbRequest *,
    enum UsbRequestStatus);
/*----------------------------------------------------------------------------*/
static bool enqueueUsbRx(struct MscQueryHandler *handler,
    uintptr_t buffer, size_t length, UsbRequestCallback silent,
    UsbRequestCallback last, size_t *queued)
{
  struct Msc * const driver = handler->driver;
  uintptr_t position = buffer;

  usbTrace("msc: OUT %"PRIu32, length);

  while (!pointerArrayEmpty(&handler->usbPool) && length)
  {
    struct UsbRequest * const request = pointerArrayBack(&handler->usbPool);
    pointerArrayPopBack(&handler->usbPool);

    const size_t prepared = prepareDataRx(handler, request, position, length,
        silent, last);

    if (usbEpEnqueue(driver->rxEp, request) == E_OK)
    {
      length -= prepared;
      position += prepared;
    }
    else
    {
      /* Hardware error occurred */
      pointerArrayPushBack(&handler->usbPool, request);
      return false;
    }
  }

  if (queued)
    *queued = position - buffer;
  return true;
}
/*----------------------------------------------------------------------------*/
static bool enqueueUsbRxRequests(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  if (query->length == query->offset)
    return true;

  size_t enqueuedBytes;

  if (enqueueUsbRx(handler, query->data + query->offset, query->length
      - query->offset, usbRxSilentCallback, usbRxLastCallback, &enqueuedBytes))
  {
    query->offset += enqueuedBytes;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static bool enqueueUsbTx(struct MscQueryHandler *handler,
    uintptr_t buffer, size_t length, UsbRequestCallback silent,
    UsbRequestCallback last, size_t *queued)
{
  struct Msc * const driver = handler->driver;
  uintptr_t position = buffer;

  usbTrace("msc: IN %"PRIu32, length);

  while (!pointerArrayEmpty(&handler->usbPool) && length)
  {
    struct UsbRequest * const request = pointerArrayBack(&handler->usbPool);
    pointerArrayPopBack(&handler->usbPool);

    const size_t prepared = prepareDataTx(handler, request, position, length,
        silent, last);

    if (usbEpEnqueue(driver->txEp, request) == E_OK)
    {
      length -= prepared;
      position += prepared;
    }
    else
    {
      /* Hardware error occurred */
      pointerArrayPushBack(&handler->usbPool, request);
      return false;
    }
  }

  if (queued)
    *queued = position - buffer;
  return true;
}
/*----------------------------------------------------------------------------*/
static bool enqueueUsbTxRequests(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  if (query->length == query->offset)
    return true;

  size_t enqueuedBytes;

  if (enqueueUsbTx(handler, query->data + query->offset, query->length
      - query->offset, usbTxSilentCallback, usbTxLastCallback, &enqueuedBytes))
  {
    query->offset += enqueuedBytes;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void fillStorageReadQueue(struct MscQueryHandler *handler)
{
  while (handler->currentQueryLength && !pointerArrayEmpty(&handler->queryPool))
  {
    struct MscQuery * const transfer = pointerArrayBack(&handler->queryPool);
    pointerArrayPopBack(&handler->queryPool);

    transfer->position = handler->currentQueryPosition;
    transfer->length = MIN(transfer->capacity, handler->currentQueryLength);
    transfer->offset = 0;

    handler->currentQueryLength -= transfer->length;
    handler->currentQueryPosition += transfer->length;

    pointerQueuePushBack(&handler->storageQueries, transfer);
  }
}
/*----------------------------------------------------------------------------*/
static void fillUsbReadQueue(struct MscQueryHandler *handler)
{
  while (handler->currentQueryLength && !pointerArrayEmpty(&handler->queryPool))
  {
    struct MscQuery * const transfer = pointerArrayBack(&handler->queryPool);
    pointerArrayPopBack(&handler->queryPool);

    transfer->position = handler->currentQueryPosition;
    transfer->length = MIN(transfer->capacity, handler->currentQueryLength);
    transfer->offset = 0;

    handler->currentQueryLength -= transfer->length;
    handler->currentQueryPosition += transfer->length;

    pointerQueuePushBack(&handler->usbQueries, transfer);
  }
}
/*----------------------------------------------------------------------------*/
static void handleIncomingFlow(struct MscQueryHandler *handler)
{
  fillUsbReadQueue(handler);

  if (!pointerQueueEmpty(&handler->usbQueries))
  {
    struct MscQuery * const query = pointerQueueFront(&handler->usbQueries);

    if (!enqueueUsbRxRequests(handler, query))
    {
      /* Transfer failed, notify parent FSM */
      handler->currentStatus = E_INTERFACE;
      handler->trampoline(handler->driver);
      return;
    }
  }

  if (!pointerQueueEmpty(&handler->storageQueries))
  {
    struct MscQuery * const query = pointerQueueFront(&handler->storageQueries);

    if (query->offset != query->length)
    {
      if (storageWrite(handler, query))
      {
        query->offset = query->length;
      }
      else
      {
        /* Transfer failed, notify parent FSM */
        handler->currentStatus = E_INTERFACE;
        handler->trampoline(handler->driver);
        return;
      }
    }
  }

  if (pointerArrayFull(&handler->queryPool) && !handler->currentQueryLength)
  {
    /* Transfer completed, invoke parent FSM */
    handler->currentStatus = E_OK;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void handleOutgoingFlow(struct MscQueryHandler *handler)
{
  fillStorageReadQueue(handler);

  if (!pointerQueueEmpty(&handler->usbQueries))
  {
    struct MscQuery * const query = pointerQueueFront(&handler->usbQueries);

    if (!enqueueUsbTxRequests(handler, query))
    {
      /* Transfer failed, notify parent FSM */
      handler->currentStatus = E_INTERFACE;
      handler->trampoline(handler->driver);
      return;
    }
  }

  if (!pointerQueueEmpty(&handler->storageQueries))
  {
    struct MscQuery * const query = pointerQueueFront(&handler->storageQueries);

    if (query->offset != query->length)
    {
      if (storageRead(handler, query))
      {
        query->offset = query->length;
      }
      else
      {
        /* Transfer failed, notify parent FSM */
        handler->currentStatus = E_INTERFACE;
        handler->trampoline(handler->driver);
        return;
      }
    }
  }

  if (pointerArrayFull(&handler->queryPool) && !handler->currentQueryLength)
  {
    /* Transfer completed, invoke parent FSM */
    handler->currentStatus = E_OK;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void makePingPongTransfer(struct MscQueryHandler *handler,
    void *buffer, size_t bufferLength, uint64_t storagePosition,
    size_t transferLength)
{
  resetTransferPool(handler);
  handler->currentQueryLength = transferLength;
  handler->currentQueryPosition = storagePosition;

  const size_t transferChunkLength = bufferLength / 2;

  handler->queries[0] = (struct MscQuery){
      .data = (uintptr_t)buffer,
      .capacity = transferChunkLength,
      .length = 0
  };
  handler->queries[1] = (struct MscQuery){
      .data = (uintptr_t)buffer + transferChunkLength,
      .capacity = transferChunkLength,
      .length = 0
  };

  pointerArrayPushBack(&handler->queryPool, &handler->queries[0]);
  pointerArrayPushBack(&handler->queryPool, &handler->queries[1]);
}
/*----------------------------------------------------------------------------*/
static size_t prepareDataRx(struct MscQueryHandler *handler,
    struct UsbRequest *request, uintptr_t buffer, size_t left,
    UsbRequestCallback silent, UsbRequestCallback last)
{
  const size_t length = MIN(left, handler->driver->packetSize);

  request->buffer = (uint8_t *)buffer;
  request->capacity = length;
  request->callbackArgument = handler;
  request->callback = (last && length == left) ? last : silent;

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t prepareDataTx(struct MscQueryHandler *handler,
    struct UsbRequest *request, uintptr_t buffer, size_t left,
    UsbRequestCallback silent, UsbRequestCallback last)
{
  const size_t length = MIN(left, handler->driver->packetSize);

  request->buffer = (uint8_t *)buffer;
  request->capacity = length;
  request->length = length;
  request->callbackArgument = handler;
  request->callback = (last && length == left) ? last : silent;

  return length;
}
/*----------------------------------------------------------------------------*/
static void resetTransferPool(struct MscQueryHandler *handler)
{
  pointerArrayClear(&handler->queryPool);
  pointerQueueClear(&handler->storageQueries);
  pointerQueueClear(&handler->usbQueries);

  handler->currentQueryLength = 0;
  handler->currentQueryPosition = 0;
  handler->currentStatus = E_BUSY;
}
/*----------------------------------------------------------------------------*/
static bool storageRead(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  struct Interface * const storage = handler->driver->storage;

  usbTrace("msc: read storage block %"PRIu32", count %"PRIu32,
      (uint32_t)(query->position / handler->driver->blockLength),
      (uint32_t)(query->length / handler->driver->blockLength));

  ifSetCallback(storage, storageReadCallback, handler);

  if (ifSetParam(storage, IF_POSITION_64, &query->position) != E_OK)
    return false;

  return ifRead(storage, (void *)query->data, query->length) == query->length;
}
/*----------------------------------------------------------------------------*/
static void storageReadCallback(void *argument)
{
  struct MscQueryHandler * const handler = argument;
  const IrqState state = irqSave();

  if (ifGetParam(handler->driver->storage, IF_STATUS, 0) != E_OK)
  {
    usbTrace("msc: storage read failed");

    /* Transfer failed, notify parent FSM */
    handler->currentStatus = E_INTERFACE;
    handler->trampoline(handler->driver);
  }
  else
  {
    usbTrace("msc: storage read done");

    struct MscQuery * const transfer =
        pointerQueueFront(&handler->storageQueries);
    pointerQueuePopFront(&handler->storageQueries);

    transfer->offset = 0;
    pointerQueuePushBack(&handler->usbQueries, transfer);

    /* Handle USB transfers */
    handleOutgoingFlow(handler);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static bool storageWrite(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  struct Interface * const storage = handler->driver->storage;

  usbTrace("msc: write storage block %"PRIu32", count %"PRIu32,
      (uint32_t)(query->position / handler->driver->blockLength),
      (uint32_t)(query->length / handler->driver->blockLength));

  ifSetCallback(storage, storageWriteCallback, handler);

  if (ifSetParam(storage, IF_POSITION_64, &query->position) != E_OK)
    return false;

  return ifWrite(storage, (const void *)query->data, query->length) ==
      query->length;
}
/*----------------------------------------------------------------------------*/
static void storageWriteCallback(void *argument)
{
  struct MscQueryHandler * const handler = argument;
  const IrqState state = irqSave();

  if (ifGetParam(handler->driver->storage, IF_STATUS, 0) != E_OK)
  {
    usbTrace("msc: storage write failed");

    /* Transfer failed, notify parent FSM */
    handler->currentStatus = E_INTERFACE;
    handler->trampoline(handler->driver);
  }
  else
  {
    usbTrace("msc: storage write done");

    struct MscQuery * const transfer =
        pointerQueueFront(&handler->storageQueries);
    pointerQueuePopFront(&handler->storageQueries);

    /* Return query to pool */
    pointerArrayPushBack(&handler->queryPool, transfer);

    /* Enqueue storage interface requests and handle USB transfers */
    handleIncomingFlow(handler);
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void usbControlCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  if (status == USB_REQUEST_COMPLETED)
  {
    usbTrace("msc: control OUT done");

    handler->currentStatus = request->length == request->capacity ?
        E_OK : E_VALUE;
    pointerArrayPushBack(&handler->usbPool, request);
    handler->trampoline(handler->driver);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    pointerArrayPushBack(&handler->usbPool, request);

    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void usbRxLastCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  pointerArrayPushBack(&handler->usbPool, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    usbTrace("msc: OUT done");

    struct MscQuery * const transfer = pointerQueueFront(&handler->usbQueries);
    pointerQueuePopFront(&handler->usbQueries);

    transfer->offset = 0;
    pointerQueuePushBack(&handler->storageQueries, transfer);
    handleIncomingFlow(handler);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void usbRxSilentCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  pointerArrayPushBack(&handler->usbPool, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t watermark = pointerArrayCapacity(&handler->usbPool) >> 1;

    if (pointerArraySize(&handler->usbPool) >= watermark)
      handleIncomingFlow(handler);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void usbTxLastCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  pointerArrayPushBack(&handler->usbPool, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    usbTrace("msc: IN done");

    struct MscQuery * const transfer = pointerQueueFront(&handler->usbQueries);
    pointerQueuePopFront(&handler->usbQueries);

    pointerArrayPushBack(&handler->queryPool, transfer);
    handleOutgoingFlow(handler);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void usbTxSilentCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  pointerArrayPushBack(&handler->usbPool, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t watermark = pointerArrayCapacity(&handler->usbPool) >> 1;

    if (pointerArraySize(&handler->usbPool) >= watermark)
      handleOutgoingFlow(handler);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
enum Result datapathInit(struct MscQueryHandler *handler,
    struct Msc *driver, void (*trampoline)(struct Msc *))
{
  if (!pointerArrayInit(&handler->usbPool, DATA_QUEUE_SIZE))
    return E_MEMORY;
  if (!pointerQueueInit(&handler->usbQueue, DATA_QUEUE_SIZE))
    return E_MEMORY;

  for (size_t index = 0; index < DATA_QUEUE_SIZE; ++index)
  {
    struct UsbRequest * const request = &handler->headers[index];

    usbRequestInit(request, 0, 0, 0, 0);
    pointerArrayPushBack(&handler->usbPool, request);
  }

  handler->driver = driver;
  handler->trampoline = trampoline;

  if (!pointerArrayInit(&handler->queryPool, 2))
    return E_MEMORY;
  if (!pointerQueueInit(&handler->storageQueries, 2))
    return E_MEMORY;
  if (!pointerQueueInit(&handler->usbQueries, 2))
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void datapathDeinit(struct MscQueryHandler *handler)
{
  pointerQueueDeinit(&handler->usbQueries);
  pointerQueueDeinit(&handler->storageQueries);
  pointerArrayDeinit(&handler->queryPool);

  pointerQueueDeinit(&handler->usbQueue);
  pointerArrayDeinit(&handler->usbPool);
}
/*----------------------------------------------------------------------------*/
enum Result datapathStatus(const struct MscQueryHandler *handler)
{
  return handler->currentStatus;
}
/*----------------------------------------------------------------------------*/
bool datapathReceiveControl(struct MscQueryHandler *handler, void *buffer,
    size_t length)
{
  return enqueueUsbRx(handler, (uintptr_t)buffer, length,
      usbControlCallback, 0, 0);
}
/*----------------------------------------------------------------------------*/
bool datapathSendResponseAndStatus(struct MscQueryHandler *handler,
    const void *buffer, size_t length, uint32_t tag, uint32_t residue,
    uint8_t status)
{
  resetTransferPool(handler);

  handler->csw = (struct CSW){
      .signature = CSW_SIGNATURE,
      .tag = tag,
      .dataResidue = toLittleEndian32(residue),
      .status = status
  };

  handler->queries[0] = (struct MscQuery){
      .position = 0,
      .data = (uintptr_t)buffer,
      .capacity = length,
      .length = length,
      .offset = 0
  };
  handler->queries[1] = (struct MscQuery){
      .position = 0,
      .data = (uintptr_t)&handler->csw,
      .capacity = sizeof(handler->csw),
      .length = sizeof(handler->csw),
      .offset = 0
  };

  pointerQueuePushBack(&handler->usbQueries, &handler->queries[0]);
  pointerQueuePushBack(&handler->usbQueries, &handler->queries[1]);

  return enqueueUsbTxRequests(handler, &handler->queries[0]);
}
/*----------------------------------------------------------------------------*/
bool datapathSendResponse(struct MscQueryHandler *handler,
    const void *buffer, size_t length)
{
  resetTransferPool(handler);

  handler->queries[0] = (struct MscQuery){
      .position = 0,
      .data = (uintptr_t)buffer,
      .capacity = length,
      .length = length,
      .offset = 0
  };
  handler->queries[1] = (struct MscQuery){
      .position = 0,
      .data = 0,
      .capacity = 0,
      .length = 0,
      .offset = 0
  };

  pointerQueuePushBack(&handler->usbQueries, &handler->queries[0]);
  pointerArrayPushBack(&handler->queryPool, &handler->queries[1]);

  return enqueueUsbTxRequests(handler, &handler->queries[0]);
}
/*----------------------------------------------------------------------------*/
bool datapathSendStatus(struct MscQueryHandler *handler,
    uint32_t tag, uint32_t residue, uint8_t status)
{
  resetTransferPool(handler);

  handler->csw = (struct CSW){
      .signature = CSW_SIGNATURE,
      .tag = tag,
      .dataResidue = toLittleEndian32(residue),
      .status = status
  };

  handler->queries[0] =
      (struct MscQuery){
      .position = 0,
      .data = (uintptr_t)&handler->csw,
      .capacity = sizeof(handler->csw),
      .length = sizeof(handler->csw),
      .offset = 0
  };
  handler->queries[1] = (struct MscQuery){
      .position = 0,
      .data = 0,
      .capacity = 0,
      .length = 0,
      .offset = 0
  };

  pointerQueuePushBack(&handler->usbQueries, &handler->queries[0]);
  pointerArrayPushBack(&handler->queryPool, &handler->queries[1]);

  return enqueueUsbTxRequests(handler, &handler->queries[0]);
}
/*----------------------------------------------------------------------------*/
bool datapathReceiveAndWriteData(struct MscQueryHandler *handler,
    void *buffer, size_t bufferLength, uint64_t storagePosition,
    size_t transferLength)
{
  makePingPongTransfer(handler, buffer, bufferLength,
      storagePosition, transferLength);
  fillUsbReadQueue(handler);

  return enqueueUsbRxRequests(handler, pointerQueueFront(&handler->usbQueries));
}
/*----------------------------------------------------------------------------*/
bool datapathReadAndSendData(struct MscQueryHandler *handler,
    void *buffer, size_t bufferLength, uint64_t storagePosition,
    size_t transferLength)
{
  makePingPongTransfer(handler, buffer, bufferLength,
      storagePosition, transferLength);
  fillStorageReadQueue(handler);

  struct MscQuery * const query = pointerQueueFront(&handler->storageQueries);

  /* Mark query as pending */
  query->offset = query->length;

  return storageRead(handler, query);
}
