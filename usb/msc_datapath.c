/*
 * msc_datapath.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <inttypes.h>
#include <halm/irq.h>
#include <halm/usb/msc.h>
#include <halm/usb/msc_datapath.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static size_t enqueueUsbRx(struct MscQueryHandler *, uintptr_t, size_t,
    UsbRequestCallback, UsbRequestCallback);
static bool enqueueUsbRxRequests(struct MscQueryHandler *, struct MscQuery *);
static size_t enqueueUsbTx(struct MscQueryHandler *, uintptr_t, size_t,
    UsbRequestCallback, UsbRequestCallback);
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
static size_t enqueueUsbRx(struct MscQueryHandler *handler,
    uintptr_t buffer, size_t length, UsbRequestCallback silent,
    UsbRequestCallback last)
{
  struct Msc * const driver = handler->driver;
  uintptr_t position = buffer;

  usbTrace("msc: OUT %"PRIu32" bytes", length);

  while (!arrayEmpty(&handler->usbPool) && length)
  {
    struct UsbRequest *request;
    arrayPopBack(&handler->usbPool, &request);

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
      arrayPushBack(&handler->usbPool, &request);
      return 0;
    }
  }

  return position - buffer;
}
/*----------------------------------------------------------------------------*/
static bool enqueueUsbRxRequests(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  if (query->length == query->offset)
    return true;

  const size_t enqueuedBytes = enqueueUsbRx(handler,
      query->data + query->offset, query->length - query->offset,
      usbRxSilentCallback, usbRxLastCallback);

  if (enqueuedBytes)
  {
    query->offset += enqueuedBytes;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static size_t enqueueUsbTx(struct MscQueryHandler *handler,
    uintptr_t buffer, size_t length, UsbRequestCallback silent,
    UsbRequestCallback last)
{
  struct Msc * const driver = handler->driver;
  uintptr_t position = buffer;

  usbTrace("msc: IN %"PRIu32" bytes", length);

  while (!arrayEmpty(&handler->usbPool) && length)
  {
    struct UsbRequest *request;
    arrayPopBack(&handler->usbPool, &request);

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
      arrayPushBack(&handler->usbPool, &request);
      return 0;
    }
  }

  return position - buffer;
}
/*----------------------------------------------------------------------------*/
static bool enqueueUsbTxRequests(struct MscQueryHandler *handler,
    struct MscQuery *query)
{
  if (query->length == query->offset)
    return true;

  const size_t enqueuedBytes = enqueueUsbTx(handler,
      query->data + query->offset, query->length - query->offset,
      usbTxSilentCallback, usbTxLastCallback);

  if (enqueuedBytes)
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
  while (handler->currentQueryLength && !arrayEmpty(&handler->queryPool))
  {
    struct MscQuery *transfer;
    arrayPopBack(&handler->queryPool, &transfer);

    transfer->position = handler->currentQueryPosition;
    transfer->length = MIN(transfer->capacity, handler->currentQueryLength);
    transfer->offset = 0;

    handler->currentQueryLength -= transfer->length;
    handler->currentQueryPosition += transfer->length;

    queuePush(&handler->storageQueries, &transfer);
  }
}
/*----------------------------------------------------------------------------*/
static void fillUsbReadQueue(struct MscQueryHandler *handler)
{
  while (handler->currentQueryLength && !arrayEmpty(&handler->queryPool))
  {
    struct MscQuery *transfer;
    arrayPopBack(&handler->queryPool, &transfer);

    transfer->position = handler->currentQueryPosition;
    transfer->length = MIN(transfer->capacity, handler->currentQueryLength);
    transfer->offset = 0;

    handler->currentQueryLength -= transfer->length;
    handler->currentQueryPosition += transfer->length;

    queuePush(&handler->usbQueries, &transfer);
  }
}
/*----------------------------------------------------------------------------*/
static void handleIncomingFlow(struct MscQueryHandler *handler)
{
  fillUsbReadQueue(handler);

  if (!queueEmpty(&handler->usbQueries))
  {
    struct MscQuery *query;
    queuePeek(&handler->usbQueries, &query);

    if (!enqueueUsbRxRequests(handler, query))
    {
      /* Transfer failed, notify parent FSM */
      handler->currentStatus = E_INTERFACE;
      handler->trampoline(handler->driver);
    }
  }

  if (!queueEmpty(&handler->storageQueries))
  {
    struct MscQuery *query;
    queuePeek(&handler->storageQueries, &query);

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
      }
    }
  }

  if (arrayFull(&handler->queryPool) && !handler->currentQueryLength)
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

  if (!queueEmpty(&handler->usbQueries))
  {
    struct MscQuery *query;
    queuePeek(&handler->usbQueries, &query);

    if (!enqueueUsbTxRequests(handler, query))
    {
      /* Transfer failed, notify parent FSM */
      handler->currentStatus = E_INTERFACE;
      handler->trampoline(handler->driver);
    }
  }

  if (arrayFull(&handler->queryPool) && !handler->currentQueryLength)
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

  const struct MscQuery * const transfers[] = {
      &handler->queries[0],
      &handler->queries[1]
  };

  arrayPushBack(&handler->queryPool, &transfers[0]);
  arrayPushBack(&handler->queryPool, &transfers[1]);
}
/*----------------------------------------------------------------------------*/
static size_t prepareDataRx(struct MscQueryHandler *handler,
    struct UsbRequest *request, uintptr_t buffer, size_t left,
    UsbRequestCallback silent, UsbRequestCallback last)
{
  const size_t length = MIN(left, handler->driver->packetSize);

  request->buffer = (uint8_t *)buffer;
  request->capacity = length;
  request->length = 0;
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
  arrayClear(&handler->queryPool);
  queueClear(&handler->storageQueries);
  queueClear(&handler->usbQueries);

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

  if (ifSetParam(storage, IF_POSITION, &query->position) != E_OK)
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

    struct MscQuery *transfer;

    queuePop(&handler->storageQueries, &transfer);
    transfer->offset = 0;
    queuePush(&handler->usbQueries, &transfer);

    /* Process next query if available */
    if (!queueEmpty(&handler->storageQueries))
    {
      queuePeek(&handler->storageQueries, &transfer);
      storageRead(handler, transfer);
    }

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

  if (ifSetParam(storage, IF_POSITION, &query->position) != E_OK)
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

    struct MscQuery *transfer;

    /* Return query to pool */
    queuePop(&handler->storageQueries, &transfer);
    arrayPushBack(&handler->queryPool, &transfer);

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
    arrayPushBack(&handler->usbPool, &request);
    handler->trampoline(handler->driver);
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    arrayPushBack(&handler->usbPool, &request);

    handler->currentStatus = E_ERROR;
    handler->trampoline(handler->driver);
  }
}
/*----------------------------------------------------------------------------*/
static void usbRxLastCallback(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct MscQueryHandler * const handler = argument;

  arrayPushBack(&handler->usbPool, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    usbTrace("msc: OUT done");

    struct MscQuery *transfer;

    queuePop(&handler->usbQueries, &transfer);
    transfer->offset = 0;
    queuePush(&handler->storageQueries, &transfer);

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

  arrayPushBack(&handler->usbPool, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t watermark = arrayCapacity(&handler->usbPool) >> 1;

    if (arraySize(&handler->usbPool) >= watermark)
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

  arrayPushBack(&handler->usbPool, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    usbTrace("msc: IN done");

    struct MscQuery *transfer;

    queuePop(&handler->usbQueries, &transfer);
    arrayPushBack(&handler->queryPool, &transfer);

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

  arrayPushBack(&handler->usbPool, &request);

  if (status == USB_REQUEST_COMPLETED)
  {
    const size_t watermark = arrayCapacity(&handler->usbPool) >> 1;

    if (arraySize(&handler->usbPool) >= watermark)
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
  enum Result res;

  res = arrayInit(&handler->usbPool, sizeof(struct UsbRequest *),
      DATA_QUEUE_SIZE);
  if (res != E_OK)
    return res;
  res = queueInit(&handler->usbQueue, sizeof(struct UsbRequest *),
      DATA_QUEUE_SIZE);
  if (res != E_OK)
    return res;

  for (size_t index = 0; index < DATA_QUEUE_SIZE; ++index)
  {
    struct UsbRequest * const request = &handler->headers[index];

    usbRequestInit(request, 0, 0, 0, 0);
    arrayPushBack(&handler->usbPool, &request);
  }

  handler->driver = driver;
  handler->trampoline = trampoline;

  res = arrayInit(&handler->queryPool, sizeof(struct MscQuery *), 2);
  if (res != E_OK)
    return res;
  res = queueInit(&handler->storageQueries, sizeof(struct MscQuery *), 2);
  if (res != E_OK)
    return res;
  res = queueInit(&handler->usbQueries, sizeof(struct MscQuery *), 2);
  if (res != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void datapathDeinit(struct MscQueryHandler *handler)
{
  queueDeinit(&handler->usbQueries);
  queueDeinit(&handler->storageQueries);
  arrayDeinit(&handler->queryPool);

  queueDeinit(&handler->usbQueue);
  arrayDeinit(&handler->usbPool);
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
      usbControlCallback, 0) == length;
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

  struct MscQuery * const transfers[] = {
      &handler->queries[0],
      &handler->queries[1]
  };

  queuePush(&handler->usbQueries, &transfers[0]);
  queuePush(&handler->usbQueries, &transfers[1]);

  return enqueueUsbTxRequests(handler, transfers[0]);
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

  struct MscQuery * const transfers[] = {
      &handler->queries[0],
      &handler->queries[1]
  };

  queuePush(&handler->usbQueries, &transfers[0]);
  arrayPushBack(&handler->queryPool, &transfers[1]);

  return enqueueUsbTxRequests(handler, transfers[0]);
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

  struct MscQuery * const transfers[] = {
      &handler->queries[0],
      &handler->queries[1]
  };

  queuePush(&handler->usbQueries, &transfers[0]);
  arrayPushBack(&handler->queryPool, &transfers[1]);

  return enqueueUsbTxRequests(handler, transfers[0]);
}
/*----------------------------------------------------------------------------*/
bool datapathReceiveAndWriteData(struct MscQueryHandler *handler,
    void *buffer, size_t bufferLength, uint64_t storagePosition,
    size_t transferLength)
{
  makePingPongTransfer(handler, buffer, bufferLength,
      storagePosition, transferLength);
  fillUsbReadQueue(handler);

  struct MscQuery *query;
  queuePeek(&handler->usbQueries, &query);

  return enqueueUsbRxRequests(handler, query);
}
/*----------------------------------------------------------------------------*/
bool datapathReadAndSendData(struct MscQueryHandler *handler,
    void *buffer, size_t bufferLength, uint64_t storagePosition,
    size_t transferLength)
{
  makePingPongTransfer(handler, buffer, bufferLength,
      storagePosition, transferLength);
  fillStorageReadQueue(handler);

  struct MscQuery *query;
  queuePeek(&handler->storageQueries, &query);

  return storageRead(handler, query);
}
