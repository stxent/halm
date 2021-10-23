/*
 * cdc_acm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/irq.h>
#include <halm/usb/cdc_acm.h>
#include <halm/usb/cdc_acm_base.h>
#include <halm/usb/cdc_acm_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct CdcAcm
{
  struct Interface base;

  /* Lower half of the driver */
  struct CdcAcmBase *driver;

  void (*callback)(void *);
  void *callbackArgument;

  /* Queue for OUT requests */
  PointerQueue rxRequestQueue;
  /* Pool for IN requests */
  PointerArray txRequestPool;
  /* Pointer to the beginning of the request pool */
  void *requests;
  /* Number of available bytes */
  size_t queuedRxBytes;
  /* Number of pending bytes */
  size_t queuedTxBytes;

  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;
  struct UsbEndpoint *notificationEp;

  /* Device suspended due to error or external request */
  bool suspended;
  /* Link configuration message received */
  bool updated;
};
/*----------------------------------------------------------------------------*/
static void cdcDataReceived(void *, struct UsbRequest *, enum UsbRequestStatus);
static void cdcDataSent(void *, struct UsbRequest *, enum UsbRequestStatus);
static inline size_t getMaxPacketSize(void);
static inline size_t getPacketSize(const struct CdcAcm *);
static bool resetEndpoints(struct CdcAcm *);
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *, const void *);
static void interfaceDeinit(void *);
static void interfaceSetCallback(void *, void (*)(void *), void *);
static enum Result interfaceGetParam(void *, int, void *);
static enum Result interfaceSetParam(void *, int, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const CdcAcm = &(const struct InterfaceClass){
    .size = sizeof(struct CdcAcm),
    .init = interfaceInit,
    .deinit = interfaceDeinit,

    .setCallback = interfaceSetCallback,
    .getParam = interfaceGetParam,
    .setParam = interfaceSetParam,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
static void cdcDataReceived(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct CdcAcm * const interface = argument;
  bool event = false;

  pointerQueuePushBack(&interface->rxRequestQueue, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    interface->queuedRxBytes += request->length;
    event = true;
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    interface->suspended = true;
    usbTrace("cdc_acm: suspended in read callback");
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void cdcDataSent(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct CdcAcm * const interface = argument;
  const size_t maxPacketSize = getPacketSize(interface);
  bool error = false;
  bool returnToPool = false;

  if (status == USB_REQUEST_CANCELLED)
  {
    returnToPool = true;
  }
  else if (status != USB_REQUEST_COMPLETED)
  {
    error = true;
  }
  else
  {
    interface->queuedTxBytes -= request->length;

    if (!interface->queuedTxBytes && request->length == maxPacketSize)
    {
      /* Send empty packet to finalize data transfer */
      request->length = 0;

      if (usbEpEnqueue(interface->txDataEp, request) != E_OK)
        error = true;
    }
    else
    {
      returnToPool = true;
    }
  }

  if (returnToPool || error)
  {
    pointerArrayPushBack(&interface->txRequestPool, request);
  }

  if (error)
  {
    interface->suspended = true;
    usbTrace("cdc_acm: suspended in write callback");
  }
  else if (pointerArrayFull(&interface->txRequestPool))
  {
    /* Notify when all data has been sent */
    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static inline size_t getMaxPacketSize(void)
{
#ifdef CONFIG_USB_DEVICE_HS
  return CDC_DATA_EP_SIZE_HS;
#else
  return CDC_DATA_EP_SIZE;
#endif
}
/*----------------------------------------------------------------------------*/
static inline size_t getPacketSize(const struct CdcAcm *interface)
{
  return cdcAcmBaseGetUsbSpeed(interface->driver) == USB_HS ?
      CDC_DATA_EP_SIZE_HS : CDC_DATA_EP_SIZE;
}
/*----------------------------------------------------------------------------*/
static bool resetEndpoints(struct CdcAcm *interface)
{
  bool completed = true;

  /* Return queued requests to pools */
  usbEpClear(interface->rxDataEp);
  usbEpClear(interface->txDataEp);

  interface->suspended = true;
  interface->queuedRxBytes = 0;
  interface->queuedTxBytes = 0;

  /* Enable endpoints */
  const size_t maxPacketSize = getPacketSize(interface);

  usbEpEnable(interface->notificationEp, ENDPOINT_TYPE_INTERRUPT,
      CDC_NOTIFICATION_EP_SIZE);
  usbEpEnable(interface->txDataEp, ENDPOINT_TYPE_BULK, maxPacketSize);
  usbEpEnable(interface->rxDataEp, ENDPOINT_TYPE_BULK, maxPacketSize);

  /* Fill OUT endpoint queue */
  while (!pointerQueueEmpty(&interface->rxRequestQueue))
  {
    struct UsbRequest * const request =
        pointerQueueFront(&interface->rxRequestQueue);
    pointerQueuePopFront(&interface->rxRequestQueue);

    if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
    {
      completed = false;
      pointerQueuePushBack(&interface->rxRequestQueue, request);
      break;
    }
  }

  if (completed)
    interface->suspended = false;

  return completed;
}
/*----------------------------------------------------------------------------*/
void cdcAcmOnParametersChanged(struct CdcAcm *interface)
{
  interface->updated = true;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
void cdcAcmOnEvent(struct CdcAcm *interface, unsigned int event)
{
  switch ((enum UsbDeviceEvent)event)
  {
    case USB_DEVICE_EVENT_RESET:
      if (resetEndpoints(interface))
      {
        interface->suspended = false;
        usbTrace("cdc_acm: reset completed");
      }
      else
        usbTrace("cdc_acm: reset failed");
      break;

    case USB_DEVICE_EVENT_SUSPEND:
      interface->suspended = true;
      usbTrace("cdc_acm: suspended");
      break;

    case USB_DEVICE_EVENT_RESUME:
      interface->suspended = false;
      usbTrace("cdc_acm: resumed");
      break;

    default:
      break;
  }

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *object, const void *configBase)
{
  const struct CdcAcmConfig * const config = configBase;
  assert(config);
  assert(config->device);

  struct CdcAcm * const interface = object;
  const struct CdcAcmBaseConfig driverConfig = {
      .device = config->device,
      .owner = interface,
      .endpoints = {
          .interrupt = config->endpoints.interrupt,
          .rx = config->endpoints.rx,
          .tx = config->endpoints.tx
      }
  };

  if (!pointerQueueInit(&interface->rxRequestQueue, config->rxBuffers))
    return E_MEMORY;
  if (!pointerArrayInit(&interface->txRequestPool, config->txBuffers))
    return E_MEMORY;

  interface->callback = 0;
  interface->queuedRxBytes = 0;
  interface->queuedTxBytes = 0;
  interface->suspended = true;
  interface->updated = false;

  interface->notificationEp = usbDevCreateEndpoint(config->device,
      config->endpoints.interrupt);
  if (!interface->notificationEp)
    return E_ERROR;
  interface->rxDataEp = usbDevCreateEndpoint(config->device,
      config->endpoints.rx);
  if (!interface->rxDataEp)
    return E_ERROR;
  interface->txDataEp = usbDevCreateEndpoint(config->device,
      config->endpoints.tx);
  if (!interface->txDataEp)
    return E_ERROR;

  const size_t count = config->rxBuffers + config->txBuffers;
  const size_t size = getMaxPacketSize();
  uint8_t *arena;

  /* Allocate requests */
  if (config->arena)
  {
    interface->requests = malloc(count * sizeof(struct UsbRequest));
    if (!interface->requests)
      return E_MEMORY;

    arena = config->arena;
  }
  else
  {
    interface->requests = malloc(count * (sizeof(struct UsbRequest) + size));
    if (!interface->requests)
      return E_MEMORY;

    arena = (uint8_t *)interface->requests + count * sizeof(struct UsbRequest);
  }

  /* Add requests to queues */
  struct UsbRequest *request = interface->requests;
  uint8_t *payload = arena;

  for (size_t index = 0; index < config->rxBuffers; ++index)
  {
    usbRequestInit(request, payload, size, cdcDataReceived, interface);
    pointerQueuePushBack(&interface->rxRequestQueue, request);

    ++request;
    payload += size;
  }

  for (size_t index = 0; index < config->txBuffers; ++index)
  {
    usbRequestInit(request, payload, size, cdcDataSent, interface);
    pointerArrayPushBack(&interface->txRequestPool, request);

    ++request;
    payload += size;
  }

  /* Lower half of the driver should be initialized after all other parts */
  interface->driver = init(CdcAcmBase, &driverConfig);
  return interface->driver ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void interfaceDeinit(void *object)
{
  struct CdcAcm * const interface = object;

  /* Call destructor for USB driver part */
  deinit(interface->driver);

  /* Return requests from endpoint queues to local pools */
  usbEpClear(interface->notificationEp);
  usbEpClear(interface->txDataEp);
  usbEpClear(interface->rxDataEp);

  assert(pointerQueueFull(&interface->rxRequestQueue));
  assert(pointerArrayFull(&interface->txRequestPool));

  /* Free memory allocated for request buffers */
  free(interface->requests);

  /* Delete endpoints */
  deinit(interface->txDataEp);
  deinit(interface->rxDataEp);
  deinit(interface->notificationEp);

  /* Delete request queues */
  pointerArrayDeinit(&interface->txRequestPool);
  pointerQueueDeinit(&interface->rxRequestQueue);
}
/*----------------------------------------------------------------------------*/
static void interfaceSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct CdcAcm * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceGetParam(void *object, int parameter, void *data)
{
  struct CdcAcm * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      if (!interface->suspended)
      {
        const size_t buffers = pointerQueueCapacity(&interface->rxRequestQueue)
            - pointerQueueSize(&interface->rxRequestQueue);
        *(size_t *)data = buffers * getPacketSize(interface);
      }
      else
        *(size_t *)data = 0;
      return E_OK;

    case IF_RX_PENDING:
      *(size_t *)data = interface->suspended ? 0 : interface->queuedRxBytes;
      return E_OK;

    case IF_TX_AVAILABLE:
      if (!interface->suspended)
      {
        const size_t buffers = pointerArraySize(&interface->txRequestPool);
        *(size_t *)data = buffers * getPacketSize(interface);
      }
      else
        *(size_t *)data = 0;
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = interface->suspended ? 0 : interface->queuedTxBytes;
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = cdcAcmBaseGetRate(interface->driver);
      return E_OK;

    default:
      break;
  }

  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_RTS:
      *(bool *)data = (cdcAcmBaseGetState(interface->driver) & CDC_RTS) != 0;
      return E_OK;

    case IF_SERIAL_DTR:
      *(bool *)data = (cdcAcmBaseGetState(interface->driver) & CDC_DTR) != 0;
      return E_OK;

    default:
      break;
  }

  switch ((enum CdcAcmParameter)parameter)
  {
    case IF_CDC_ACM_STATUS:
    {
      uint8_t status = 0;

      if (interface->updated)
      {
        interface->updated = false;
        status |= CDC_ACM_LINE_CHANGED;
      }
      if (interface->suspended)
      {
        status |= CDC_ACM_SUSPENDED;
      }
      else
      {
        if (interface->queuedRxBytes)
          status |= CDC_ACM_RX_AVAILABLE;
        if (!interface->queuedTxBytes)
          status |= CDC_ACM_TX_EMPTY;
      }

      *(uint8_t *)data = status;
      return E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceSetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct CdcAcm * const interface = object;
  uint8_t *bufferPosition = buffer;

  assert(length >= getPacketSize(interface));

  if (interface->suspended)
    return 0;

  while (!pointerQueueEmpty(&interface->rxRequestQueue))
  {
    struct UsbRequest * const request =
        pointerQueueFront(&interface->rxRequestQueue);

    if (length < request->length)
      break;

    const size_t bytesToRead = request->length;
    IrqState state;

    state = irqSave();
    pointerQueuePopFront(&interface->rxRequestQueue);
    interface->queuedRxBytes -= bytesToRead;
    irqRestore(state);

    memcpy(bufferPosition, request->buffer, bytesToRead);
    bufferPosition += bytesToRead;
    length -= bytesToRead;

    if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the interface and wait for reset */
      interface->suspended = true;

      state = irqSave();
      pointerQueuePushBack(&interface->rxRequestQueue, request);
      irqRestore(state);

      usbTrace("cdc_acm: suspended in read function");
      break;
    }
  }

  return bufferPosition - (uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct CdcAcm * const interface = object;
  const uint8_t *bufferPosition = buffer;
  const size_t maxPacketSize = getPacketSize(interface);

  if (interface->suspended)
    return 0;

  while (length && !pointerArrayEmpty(&interface->txRequestPool))
  {
    const size_t bytesToWrite = MIN(length, maxPacketSize);
    struct UsbRequest *request;
    IrqState state;

    /* Critical section */
    state = irqSave();
    request = pointerArrayBack(&interface->txRequestPool);
    pointerArrayPopBack(&interface->txRequestPool);
    interface->queuedTxBytes += bytesToWrite;
    irqRestore(state);

    request->length = bytesToWrite;
    memcpy(request->buffer, bufferPosition, bytesToWrite);

    if (usbEpEnqueue(interface->txDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the interface and wait for reset */
      interface->suspended = true;

      state = irqSave();
      pointerArrayPushBack(&interface->txRequestPool, request);
      irqRestore(state);

      usbTrace("cdc_acm: suspended in write function");
      break;
    }

    bufferPosition += bytesToWrite;
    length -= bytesToWrite;
  }

  return bufferPosition - (const uint8_t *)buffer;
}
