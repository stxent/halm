/*
 * usb_control.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_requests.h>
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (CONFIG_USB_DEVICE_CONTROL_REQUESTS * EP0_BUFFER_SIZE)
#define REQUEST_POOL_SIZE (CONFIG_USB_DEVICE_CONTROL_REQUESTS)
/*----------------------------------------------------------------------------*/
struct ControlUsbRequest
{
  struct UsbRequest base;

  uint8_t payload[EP0_BUFFER_SIZE];
};

struct PrivateData
{
  struct ControlUsbRequest requests[REQUEST_POOL_SIZE + 1];

  struct UsbSetupPacket setupPacket;
  uint16_t setupDataLeft;
  uint8_t setupData[DATA_BUFFER_SIZE];
};
/*----------------------------------------------------------------------------*/
static enum result privateDataAllocate(struct UsbControl *);
static void privateDataFree(struct UsbControl *);
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void controlOutHandler(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void enableEndpoints(struct UsbControl *);
static void resetDevice(struct UsbControl *);
static void sendResponse(struct UsbControl *, const uint8_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result controlInit(void *, const void *);
static void controlDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass controlTable = {
    .size = sizeof(struct UsbControl),
    .init = controlInit,
    .deinit = controlDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbControl = &controlTable;
/*----------------------------------------------------------------------------*/
static enum result privateDataAllocate(struct UsbControl *control)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));

  if (!privateData)
    return E_MEMORY;
  control->privateData = privateData;

  privateData->setupDataLeft = 0;
  memset(&privateData->setupPacket, 0, sizeof(privateData->setupPacket));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void privateDataFree(struct UsbControl *control)
{
  free(control->privateData);
}
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status __attribute__((unused)))
{
  struct UsbControl * const control = argument;

  queuePush(&control->inRequestPool, &request);
}
/*----------------------------------------------------------------------------*/
static void controlOutHandler(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct UsbControl * const control = argument;
  struct PrivateData * const privateData = control->privateData;
  struct UsbSetupPacket * const packet = &privateData->setupPacket;

  if (status == USB_REQUEST_CANCELLED)
    return;

  uint16_t length = 0;
  enum result res = E_BUSY;

  if (status == USB_REQUEST_SETUP)
  {
    privateData->setupDataLeft = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      res = usbHandleStandardRequest(control, packet, privateData->setupData,
          &length, DATA_BUFFER_SIZE);

      if (res != E_OK && control->driver)
      {
        res = usbDriverConfigure(control->driver, packet, 0, 0,
            privateData->setupData, &length, DATA_BUFFER_SIZE);
      }
    }
    else if (packet->length <= DATA_BUFFER_SIZE)
    {
      privateData->setupDataLeft = packet->length;
    }
  }
  else if (privateData->setupDataLeft
      && request->length <= privateData->setupDataLeft)
  {
    /* Erroneous packets are ignored */
    memcpy(privateData->setupData + (packet->length
        - privateData->setupDataLeft), request->buffer, request->length);
    privateData->setupDataLeft -= request->length;

    if (!privateData->setupDataLeft)
    {
      res = usbDriverConfigure(control->driver, packet, privateData->setupData,
          packet->length, 0, 0, 0);
    }
  }

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(control, privateData->setupData, length);
  }
  else if (res != E_BUSY)
  {
    usbEpSetStalled(control->ep0in, true);
  }

  usbEpEnqueue(control->ep0out, request);
}
/*----------------------------------------------------------------------------*/
static void enableEndpoints(struct UsbControl *control)
{
  usbEpEnable(control->ep0in, ENDPOINT_TYPE_CONTROL, EP0_BUFFER_SIZE);
  usbEpEnable(control->ep0out, ENDPOINT_TYPE_CONTROL, EP0_BUFFER_SIZE);
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbControl *control)
{
  usbEpClear(control->ep0in);
  usbEpClear(control->ep0out);

  enableEndpoints(control);

  usbEpEnqueue(control->ep0out, control->outRequest);
}
/*----------------------------------------------------------------------------*/
static void sendResponse(struct UsbControl *control, const uint8_t *data,
    uint16_t length)
{
  struct UsbRequest *request;
  unsigned int chunkCount;

  chunkCount = length / EP0_BUFFER_SIZE + 1;
  /* Send zero-length packet to finalize transfer */
  if (length && !(length % EP0_BUFFER_SIZE))
    ++chunkCount;

  if (queueSize(&control->inRequestPool) < chunkCount)
    return;

  for (unsigned int index = 0; index < chunkCount; ++index)
  {
    queuePop(&control->inRequestPool, &request);

    const uint16_t chunk = EP0_BUFFER_SIZE < length ? EP0_BUFFER_SIZE : length;

    if (chunk)
      memcpy(request->buffer, data, chunk);
    request->length = chunk;

    data += chunk;
    length -= chunk;

    usbEpEnqueue(control->ep0in, request);
  }
}
/*----------------------------------------------------------------------------*/
enum result usbControlAppendDescriptor(struct UsbControl *control,
    const void *descriptor)
{
  return listPush(&control->descriptors, &descriptor);
}
/*----------------------------------------------------------------------------*/
void usbControlEraseDescriptor(struct UsbControl *control,
    const void *descriptor)
{
  struct ListNode * const node = listFind(&control->descriptors, &descriptor);

  if (node)
    listErase(&control->descriptors, node);
}
/*----------------------------------------------------------------------------*/
enum result usbControlBindDriver(struct UsbControl *control, void *driver)
{
  if (!driver)
    return E_VALUE;

  /* A driver is already bound */
  if (control->driver)
    return E_ERROR;

  control->driver = driver;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbControlResetDriver(struct UsbControl *control)
{
  if (control->driver)
    usbDriverEvent(control->driver, USB_DEVICE_EVENT_RESET);
}
/*----------------------------------------------------------------------------*/
void usbControlUnbindDriver(struct UsbControl *control)
{
  control->driver = 0;
}
/*----------------------------------------------------------------------------*/
void usbControlEvent(struct UsbControl *control, unsigned int event)
{
  if (event == USB_DEVICE_EVENT_RESET)
  {
    resetDevice(control);
  }
  else
  {
    if (control->driver)
      usbDriverEvent(control->driver, event);
  }
}
/*----------------------------------------------------------------------------*/
static enum result controlInit(void *object, const void *configBase)
{
  const struct UsbControlConfig * const config = configBase;
  struct UsbControl * const control = object;
  enum result res;

  if (!config->parent)
    return E_VALUE;

  control->owner = config->parent;

  /* Create control endpoints */
  control->ep0in = usbDevCreateEndpoint(control->owner, USB_EP_DIRECTION_IN
      | USB_EP_ADDRESS(0));
  if (!control->ep0in)
    return E_MEMORY;
  control->ep0out = usbDevCreateEndpoint(control->owner, USB_EP_ADDRESS(0));
  if (!control->ep0out)
    return E_MEMORY;

  /* Create a list for USB descriptors */
  res = listInit(&control->descriptors, sizeof(const struct UsbDescriptor *));
  if (res != E_OK)
    return res;

  res = privateDataAllocate(control);
  if (res != E_OK)
    return E_MEMORY;

  control->driver = 0;

  /* Initialize request queues */
  res = queueInit(&control->inRequestPool, sizeof(struct UsbRequest *),
      REQUEST_POOL_SIZE);
  if (res != E_OK)
    return res;

  /* Enable endpoints before request queuing */
  enableEndpoints(control);

  /* Initialize requests */
  struct PrivateData * const privateData = control->privateData;
  struct ControlUsbRequest *request = privateData->requests;

  for (unsigned int index = 0; index < REQUEST_POOL_SIZE; ++index)
  {
    usbRequestInit((struct UsbRequest *)request, request->payload,
        sizeof(request->payload), controlInHandler, control);
    queuePush(&control->inRequestPool, &request);
    ++request;
  }

  control->outRequest = (struct UsbRequest *)request;
  usbRequestInit(control->outRequest, request->payload,
      sizeof(request->payload), controlOutHandler, control);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void controlDeinit(void *object)
{
  struct UsbControl * const control = object;

  usbEpClear(control->ep0in);
  usbEpClear(control->ep0out);

  assert(queueSize(&control->inRequestPool) == REQUEST_POOL_SIZE);

  queueDeinit(&control->inRequestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);

  privateDataFree(control);

  assert(listEmpty(&control->descriptors));
  listDeinit(&control->descriptors);
}
