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
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (EP0_BUFFER_SIZE * CONFIG_USB_CONTROL_REQUESTS)
/*----------------------------------------------------------------------------*/
static void controlInHandler(struct UsbRequest *, void *);
static void controlOutHandler(struct UsbRequest *, void *);
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
static void controlInHandler(struct UsbRequest *request, void *argument)
{
  struct UsbControl * const control = argument;

  queuePush(&control->requestPool, &request);
}
/*----------------------------------------------------------------------------*/
static void controlOutHandler(struct UsbRequest *request,
    void *argument)
{
  struct UsbControl * const control = argument;
  struct UsbSetupPacket * const packet = &control->state.packet;

  if (request->status == REQUEST_CANCELLED)
  {
    queuePush(&control->requestPool, &request);
    return;
  }

  uint16_t length = 0;
  enum result res = E_BUSY;

  if (request->status == REQUEST_SETUP)
  {
    control->state.left = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      res = usbHandleStandardRequest(control, packet, control->state.buffer,
          &length, DATA_BUFFER_SIZE);

      if (res != E_OK && control->driver)
      {
        res = usbDriverConfigure(control->driver, packet, 0, 0,
            control->state.buffer, &length, DATA_BUFFER_SIZE);
      }
    }
    else if (packet->length <= DATA_BUFFER_SIZE)
    {
      control->state.left = packet->length;
    }
  }
  else if (control->state.left && request->length <= control->state.left)
  {
    /* Erroneous packets are ignored */
    memcpy(control->state.buffer + (packet->length - control->state.left),
        request->buffer, request->length);
    control->state.left -= request->length;

    if (!control->state.left && control->driver)
    {
      res = usbDriverConfigure(control->driver, packet,
          control->state.buffer, packet->length, 0, 0, 0);
    }
  }

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(control, control->state.buffer, length);
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
    request->length = chunk;
    request->status = 0;

    data += chunk;
    length -= chunk;

    usbEpEnqueue(control->ep0in, request);
  }
}
/*----------------------------------------------------------------------------*/
enum result usbControlSetDriver(struct UsbControl *control, void *driver)
{
  if (driver)
  {
    return compareExchangePointer((void **)&control->driver, 0, driver) ?
        E_OK : E_BUSY;
  }
  else
  {
    //TODO Check state
    control->driver = driver;
    return E_OK;
  }
}
/*----------------------------------------------------------------------------*/
void usbControlUpdateStatus(struct UsbControl *control, uint8_t status)
{
  if (status & DEVICE_STATUS_RESET)
    resetDevice(control);

  if (control->driver)
    usbDriverUpdateStatus(control->driver, status);
}
/*----------------------------------------------------------------------------*/
static enum result controlInit(void *object, const void *configBase)
{
  const struct UsbControlConfig * const config = configBase;
  struct UsbControl * const control = object;
  enum result res;

  if (!config->parent)
    return E_VALUE;

  control->base = config->parent;
  control->driver = 0;
  control->currentConfiguration = 0;

  control->state.buffer = malloc(DATA_BUFFER_SIZE);
  if (!control->state.buffer)
    return E_MEMORY;
  control->state.left = 0;

  /* Create control endpoints */
  control->ep0in = usbDevAllocate(control->base, EP_DIRECTION_IN
      | EP_ADDRESS(0));
  if (!control->ep0in)
    return E_MEMORY;
  control->ep0out = usbDevAllocate(control->base, EP_ADDRESS(0));
  if (!control->ep0out)
    return E_MEMORY;

  res = queueInit(&control->requestPool, sizeof(struct UsbRequest *),
      CONFIG_USB_CONTROL_REQUESTS * 2);
  if (res != E_OK)
    return res;

  /* Allocate requests */
  control->requests = malloc(2 * CONFIG_USB_CONTROL_REQUESTS
      * sizeof(struct UsbRequest));
  if (!control->requests)
    return E_MEMORY;

  int8_t index;

  for (index = 0; index < 2 * CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    res = usbRequestInit(control->requests + index, EP0_BUFFER_SIZE);
    if (res != E_OK)
      return res;
  }

  for (index = 0; index < CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    struct UsbRequest * const request = control->requests + index;

    usbRequestCallback(request, controlInHandler, control);
    queuePush(&control->requestPool, &request);
  }

  for (; index < 2 * CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    usbRequestCallback(control->requests + index, controlOutHandler, control);
    usbEpEnqueue(control->ep0out, control->requests + index);
  }

  /* Enable interrupts */
  resetDevice(control);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void controlDeinit(void *object)
{
  struct UsbControl * const control = object;

  usbEpClear(control->ep0in);
  usbEpClear(control->ep0out);

  assert(queueSize(&control->requestPool) == 2 * CONFIG_USB_CONTROL_REQUESTS);

  for (int8_t index = 2 * CONFIG_USB_CONTROL_REQUESTS - 1; index >= 0; --index)
    usbRequestDeinit(control->requests + index);
  free(control->requests);

  queueDeinit(&control->requestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);
  free(control->state.buffer);
}
