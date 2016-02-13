/*
 * cdc_acm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <irq.h>
#include <usb/cdc_acm.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
struct CdcUsbRequest
{
  struct UsbRequestBase base;
  uint8_t buffer[CDC_DATA_EP_SIZE];
};
/*----------------------------------------------------------------------------*/
static void cdcDataReceived(void *, struct UsbRequest *, enum usbRequestStatus);
static void cdcDataSent(void *, struct UsbRequest *, enum usbRequestStatus);
static void resetBuffers(struct CdcAcm *);
/*----------------------------------------------------------------------------*/
static enum result interfaceInit(void *, const void *);
static void interfaceDeinit(void *);
static enum result interfaceCallback(void *, void (*)(void *), void *);
static enum result interfaceGet(void *, enum ifOption, void *);
static enum result interfaceSet(void *, enum ifOption, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass interfaceTable = {
    .size = sizeof(struct CdcAcm),
    .init = interfaceInit,
    .deinit = interfaceDeinit,

    .callback = interfaceCallback,
    .get = interfaceGet,
    .set = interfaceSet,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const CdcAcm = &interfaceTable;
/*----------------------------------------------------------------------------*/
static void cdcDataReceived(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct CdcAcm * const interface = argument;
  bool event = false;

  queuePush(&interface->rxRequestQueue, &request);

  if (status == REQUEST_COMPLETED)
  {
    interface->queuedRxBytes += request->base.length;
    event = true;
  }
  else if (status != REQUEST_CANCELLED)
  {
    interface->suspended = true;
    usbTrace("cdc_acm: suspended in read callback");
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void cdcDataSent(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct CdcAcm * const interface = argument;
  bool error = false;
  bool returnToPool = false;

  if (status == REQUEST_CANCELLED)
  {
    returnToPool = true;
  }
  else if (status != REQUEST_COMPLETED)
  {
    error = true;
  }
  else
  {
    interface->queuedTxBytes -= request->base.length;

    if (interface->zeroPacketRequired)
    {
      interface->zeroPacketRequired = false;

      /* Send empty packet to finish data transfer */
      request->base.length = 0;

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
    queuePush(&interface->txRequestQueue, &request);
  }

  if (error)
  {
    interface->suspended = true;
    usbTrace("cdc_acm: suspended in write callback");
  }
  else if (queueFull(&interface->txRequestQueue))
  {
    /* Notify when the pool becomes full */
    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void resetBuffers(struct CdcAcm *interface)
{
  /* Return queued requests to pools */
  usbEpClear(interface->rxDataEp);
  usbEpClear(interface->txDataEp);

  interface->queuedRxBytes = 0;
  interface->queuedTxBytes = 0;
  interface->zeroPacketRequired = false;
}
/*----------------------------------------------------------------------------*/
void cdcAcmOnParametersChanged(struct CdcAcm *interface)
{
  interface->updated = true;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
void cdcAcmOnStatusChanged(struct CdcAcm *interface, uint8_t status)
{
  if (!interface->suspended && (status & DEVICE_STATUS_SUSPENDED))
  {
    interface->suspended = true;

    usbTrace("cdc_acm: suspended externally");
  }

  if (status & DEVICE_STATUS_RESET)
  {
    interface->suspended = true;
    resetBuffers(interface);

    while (!queueEmpty(&interface->rxRequestQueue))
    {
      struct UsbRequest *request;

      queuePop(&interface->rxRequestQueue, &request);
      request->base.length = 0;

      if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
      {
        queuePush(&interface->rxRequestQueue, &request);
        break;
      }
    }

    interface->suspended = false;
    usbTrace("cdc_acm: reset completed");
  }

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result interfaceInit(void *object, const void *configBase)
{
  const struct CdcAcmConfig * const config = configBase;
  struct CdcAcm * const interface = object;
  const struct CdcAcmBaseConfig driverConfig = {
      .device = config->device,
      .owner = interface,
      .endpoint = {
          .interrupt = config->endpoint.interrupt,
          .rx = config->endpoint.rx,
          .tx = config->endpoint.tx
      }
  };
  enum result res;

  interface->driver = init(CdcAcmBase, &driverConfig);
  if (!interface->driver)
    return E_ERROR;

  res = queueInit(&interface->rxRequestQueue, sizeof(struct UsbRequest *),
      config->rxBuffers);
  if (res != E_OK)
    return res;
  res = queueInit(&interface->txRequestQueue, sizeof(struct UsbRequest *),
      config->txBuffers);
  if (res != E_OK)
    return res;

  interface->callback = 0;
  interface->queuedRxBytes = 0;
  interface->queuedTxBytes = 0;
  interface->suspended = true;
  interface->updated = false;
  interface->zeroPacketRequired = false;

  interface->notificationEp = usbDevCreateEndpoint(config->device,
      config->endpoint.interrupt);
  if (!interface->notificationEp)
    return E_ERROR;
  interface->rxDataEp = usbDevCreateEndpoint(config->device,
      config->endpoint.rx);
  if (!interface->rxDataEp)
    return E_ERROR;
  interface->txDataEp = usbDevCreateEndpoint(config->device,
      config->endpoint.tx);
  if (!interface->txDataEp)
    return E_ERROR;

  /* Allocate requests */
  const size_t totalPoolSize = config->rxBuffers + config->txBuffers;

  interface->requests = malloc(totalPoolSize * sizeof(struct CdcUsbRequest));
  if (!interface->requests)
    return E_MEMORY;

  /* Add requests to queues */
  struct CdcUsbRequest *request = interface->requests;

  for (size_t index = 0; index < config->rxBuffers; ++index)
  {
    usbRequestInit((struct UsbRequest *)request, CDC_DATA_EP_SIZE,
        cdcDataReceived, interface);
    queuePush(&interface->rxRequestQueue, &request);
    ++request;
  }

  for (size_t index = 0; index < config->txBuffers; ++index)
  {
    usbRequestInit((struct UsbRequest *)request, CDC_DATA_EP_SIZE,
        cdcDataSent, interface);
    queuePush(&interface->txRequestQueue, &request);
    ++request;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interfaceDeinit(void *object)
{
  struct CdcAcm * const interface = object;

  /* Return requests from endpoint queues to local pools */
  usbEpClear(interface->notificationEp);
  usbEpClear(interface->txDataEp);
  usbEpClear(interface->rxDataEp);

  assert(queueFull(&interface->rxRequestQueue));
  assert(queueFull(&interface->txRequestQueue));

  /* Delete requests and free memory */
  free(interface->requests);

  /* Delete endpoints */
  deinit(interface->txDataEp);
  deinit(interface->rxDataEp);
  deinit(interface->notificationEp);

  /* Delete request queues */
  queueDeinit(&interface->txRequestQueue);
  queueDeinit(&interface->rxRequestQueue);

  /* Call destructor for USB driver part */
  deinit(interface->driver);
}
/*----------------------------------------------------------------------------*/
static enum result interfaceCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct CdcAcm * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result interfaceGet(void *object, enum ifOption option,
    void *data)
{
  struct CdcAcm * const interface = object;

  switch (option)
  {
    case IF_AVAILABLE:
      *(uint32_t *)data = interface->suspended ? 0 : interface->queuedRxBytes;
      return E_OK;

    case IF_PENDING:
      *(uint32_t *)data = interface->suspended ? 0 : interface->queuedTxBytes;
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = interface->driver->lineCoding.dteRate;
      return E_OK;

    default:
      break;
  }

  switch ((enum cdcAcmOption)option)
  {
    case IF_CDC_ACM_STATUS:
    {
      uint32_t status = 0;

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

      *(uint32_t *)data = status;
      return E_OK;
    }

    default:
      break;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result interfaceSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct CdcAcm * const interface = object;
  uint8_t *bufferPosition = buffer;
  const size_t sourceLength = length;
  irqState state;

  if (interface->suspended)
    return 0;

  while (!queueEmpty(&interface->rxRequestQueue))
  {
    struct UsbRequest *request;
    queuePeek(&interface->rxRequestQueue, &request);

    const size_t chunkLength = request->base.length;
    if (length < chunkLength)
      break;

    state = irqSave();
    queuePop(&interface->rxRequestQueue, 0);
    interface->queuedRxBytes -= chunkLength;
    irqRestore(state);

    memcpy(bufferPosition, request->buffer, chunkLength);
    bufferPosition += chunkLength;
    length -= chunkLength;

    request->base.length = 0;

    if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the interface and wait for reset */
      interface->suspended = true;

      state = irqSave();
      queuePush(&interface->rxRequestQueue, &request);
      irqRestore(state);

      usbTrace("cdc_acm: suspended in read function");
      break;
    }
  }

  return sourceLength - length;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct CdcAcm * const interface = object;
  const uint8_t *bufferPosition = buffer;
  const size_t sourceLength = length;
  irqState state;

  if (interface->suspended)
    return 0;

  while (length && !queueEmpty(&interface->txRequestQueue))
  {
    const size_t bytesToWrite = length > CDC_DATA_EP_SIZE ?
        CDC_DATA_EP_SIZE : (uint16_t)length;
    struct UsbRequest *request;

    state = irqSave();
    queuePop(&interface->txRequestQueue, &request);
    interface->queuedTxBytes += bytesToWrite;
    irqRestore(state);

    request->base.length = bytesToWrite;
    memcpy(request->buffer, bufferPosition, bytesToWrite);
    bufferPosition += bytesToWrite;
    length -= bytesToWrite;

    /* Append zero length packet when the last packet has maximum length */
    interface->zeroPacketRequired = !length && bytesToWrite == CDC_DATA_EP_SIZE;

    if (usbEpEnqueue(interface->txDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the interface and wait for reset */
      interface->suspended = true;

      state = irqSave();
      queuePush(&interface->txRequestQueue, &request);
      irqRestore(state);

      usbTrace("cdc_acm: suspended in write function");
      return 0;
    }
  }

  return sourceLength - length;
}
