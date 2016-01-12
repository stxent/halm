/*
 * usb_control.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/usb_control.h>
#include <usb/usb_requests.h>
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (CONFIG_USB_DEVICE_CONTROL_REQUESTS * EP0_BUFFER_SIZE)
#define REQUEST_POOL_SIZE (CONFIG_USB_DEVICE_CONTROL_REQUESTS * 2)
/*----------------------------------------------------------------------------*/
struct ControlUsbRequest
{
  struct UsbRequestBase base;
  uint8_t buffer[EP0_BUFFER_SIZE];
};

struct LocalData
{
  struct ControlUsbRequest requests[REQUEST_POOL_SIZE];

  struct UsbSetupPacket setupPacket;
  uint16_t setupDataLeft;
  uint8_t setupData[DATA_BUFFER_SIZE];
};
/*----------------------------------------------------------------------------*/
static enum result localDataAllocate(struct UsbControl *);
static void localDataFree(struct UsbControl *);
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *, struct UsbRequest *,
    enum usbRequestStatus);
static void controlOutHandler(void *, struct UsbRequest *,
    enum usbRequestStatus);
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
static enum result localDataAllocate(struct UsbControl *control)
{
  struct LocalData * const local = malloc(sizeof(struct LocalData));

  if (!local)
    return E_MEMORY;
  control->local = local;

  local->setupDataLeft = 0;
  memset(&local->setupPacket, 0, sizeof(local->setupPacket));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void localDataFree(struct UsbControl *control)
{
  free(control->local);
}
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status __attribute__((unused)))
{
  struct UsbControl * const control = argument;

  queuePush(&control->requestPool, &request);
}
/*----------------------------------------------------------------------------*/
static void controlOutHandler(void *argument, struct UsbRequest *request,
    enum usbRequestStatus status)
{
  struct UsbControl * const control = argument;
  struct LocalData * const local = control->local;
  struct UsbSetupPacket * const packet = &local->setupPacket;

  if (status == REQUEST_CANCELLED)
  {
    queuePush(&control->requestPool, &request);
    return;
  }

  uint16_t length = 0;
  enum result res = E_BUSY;

  if (status == REQUEST_SETUP)
  {
    local->setupDataLeft = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      res = usbHandleStandardRequest(control, packet, local->setupData,
          &length, DATA_BUFFER_SIZE);

      if (res != E_OK && control->driver)
      {
        res = usbDriverConfigure(control->driver, packet, 0, 0,
            local->setupData, &length, DATA_BUFFER_SIZE);
      }
    }
    else if (packet->length <= DATA_BUFFER_SIZE)
    {
      local->setupDataLeft = packet->length;
    }
  }
  else if (local->setupDataLeft && request->base.length <= local->setupDataLeft)
  {
    /* Erroneous packets are ignored */
    memcpy(local->setupData + (packet->length - local->setupDataLeft),
        request->buffer, request->base.length);
    local->setupDataLeft -= request->base.length;

    if (!local->setupDataLeft)
    {
      res = usbDriverConfigure(control->driver, packet, local->setupData,
          packet->length, 0, 0, 0);
    }
  }

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(control, local->setupData, length);
  }
  else if (res != E_BUSY)
  {
    usbEpSetStalled(control->ep0in, true);
  }

  usbEpEnqueue(control->ep0out, request);
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbControl *control)
{
  usbEpSetEnabled(control->ep0in, true, EP0_BUFFER_SIZE);
  usbEpSetEnabled(control->ep0out, true, EP0_BUFFER_SIZE);
}
/*----------------------------------------------------------------------------*/
static void sendResponse(struct UsbControl *control, const uint8_t *data,
    uint16_t length)
{
  struct UsbRequest *request;
  uint8_t chunkCount;

  chunkCount = length / EP0_BUFFER_SIZE + 1;
  /* Send zero-length packet to finalize transfer */
  if (length && !(length % EP0_BUFFER_SIZE))
    ++chunkCount;

  if (queueSize(&control->requestPool) < chunkCount)
    return;

  for (uint8_t index = 0; index < chunkCount; ++index)
  {
    queuePop(&control->requestPool, &request);

    const uint16_t chunk = EP0_BUFFER_SIZE < length ? EP0_BUFFER_SIZE : length;

    if (chunk)
      memcpy(request->buffer, data, chunk);
    request->base.length = chunk;

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
    usbDriverUpdateStatus(control->driver, DEVICE_STATUS_RESET);
}
/*----------------------------------------------------------------------------*/
void usbControlUnbindDriver(struct UsbControl *control)
{
  control->driver = 0;
}
/*----------------------------------------------------------------------------*/
void usbControlUpdateStatus(struct UsbControl *control, uint8_t status)
{
  if (status & DEVICE_STATUS_RESET)
    resetDevice(control);

  if (control->driver)
    usbDriverUpdateStatus(control->driver, status & ~DEVICE_STATUS_RESET);
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
  control->ep0in = usbDevAllocate(control->owner, EP_DIRECTION_IN
      | EP_ADDRESS(0));
  if (!control->ep0in)
    return E_MEMORY;
  control->ep0out = usbDevAllocate(control->owner, EP_ADDRESS(0));
  if (!control->ep0out)
    return E_MEMORY;

  /* Create a list for USB descriptors */
  res = listInit(&control->descriptors, sizeof(const struct UsbDescriptor *));
  if (res != E_OK)
    return res;

  res = localDataAllocate(control);
  if (res != E_OK)
    return E_MEMORY;

  control->driver = 0;

  /* Initialize request pool */
  res = queueInit(&control->requestPool, sizeof(struct UsbRequest *),
      REQUEST_POOL_SIZE);
  if (res != E_OK)
    return res;

  /* Enable interrupts */
  resetDevice(control);

  /* Enqueue requests after enabling of endpoints */
  struct ControlUsbRequest *request =
      ((struct LocalData *)control->local)->requests;

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE / 2; ++index)
  {
    usbRequestInit((struct UsbRequest *)request, EP0_BUFFER_SIZE,
        controlInHandler, control);
    queuePush(&control->requestPool, &request);
    ++request;

    usbRequestInit((struct UsbRequest *)request, EP0_BUFFER_SIZE,
        controlOutHandler, control);
    usbEpEnqueue(control->ep0out, (struct UsbRequest *)request);
    ++request;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void controlDeinit(void *object)
{
  struct UsbControl * const control = object;

  usbEpClear(control->ep0in);
  usbEpClear(control->ep0out);

  assert(queueSize(&control->requestPool) == REQUEST_POOL_SIZE);

  queueDeinit(&control->requestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);

  localDataFree(control);

  assert(listEmpty(&control->descriptors));
  listDeinit(&control->descriptors);
}
