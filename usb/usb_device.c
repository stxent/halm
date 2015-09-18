/*
 * usb_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/usb_device.h>
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (EP0_BUFFER_SIZE * CONFIG_USB_CONTROL_REQUESTS)
/*----------------------------------------------------------------------------*/
static void controlInHandler(struct UsbRequest *, void *);
static void controlOutHandler(struct UsbRequest *, void *);
static void processRequest(struct UsbDevice *, const struct UsbRequest *);
static void processStandardRequest(struct UsbDevice *,
    const struct UsbSetupPacket *);
static void resetDevice(struct UsbDevice *);
static void sendResponse(struct UsbDevice *, const uint8_t *, uint16_t);
static void updateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static enum result devBind(void *, void *);
static void *devAllocate(void *, uint8_t);
static void devSetAddress(void *, uint8_t);
static void devSetConfigured(void *, bool);
static void devSetConnected(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass devTable = {
    .size = sizeof(struct UsbDevice),
    .init = devInit,
    .deinit = devDeinit,

    .allocate = devAllocate,
    .bind = devBind,
    .setAddress = devSetAddress,
    .setConfigured = devSetConfigured,
    .setConnected = devSetConnected
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const UsbDevice = &devTable;
/*----------------------------------------------------------------------------*/
static void controlInHandler(struct UsbRequest *request, void *argument)
{
  struct UsbDevice * const device = argument;

  queuePush(&device->requestPool, &request);
}
/*----------------------------------------------------------------------------*/
static void controlOutHandler(struct UsbRequest *request,
    void *argument)
{
  struct UsbDevice * const device = argument;
  struct UsbSetupPacket * const packet = &device->state.packet;

  if (request->status == REQUEST_CANCELLED)
  {
    queuePush(&device->requestPool, &request);
    return;
  }

  if (request->status == REQUEST_SETUP)
  {
    device->state.left = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_STANDARD)
    {
      device->state.packet.length = 0;
      processRequest(device, request);
    }
    else
    {
      const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

      if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
      {
        processStandardRequest(device, packet);
      }
      else if (packet->length <= DATA_BUFFER_SIZE)
      {
        device->state.left = packet->length;
      }
    }
  }
  else
  {
    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_STANDARD)
    {
      processRequest(device, request);
    }
    else if (device->state.left)
    {
      /* Ingore erroneous packets */
      if (request->length > device->state.left)
        return;

      memcpy(device->state.buffer + (packet->length - device->state.left),
          request->buffer, request->length);
      device->state.left -= request->length;

      if (!device->state.left)
        processStandardRequest(device, packet);
    }
  }

  usbEpEnqueue(device->ep0out, request);
}
/*----------------------------------------------------------------------------*/
static void processRequest(struct UsbDevice *device,
    const struct UsbRequest *request)
{
  uint16_t length = DATA_BUFFER_SIZE;
  enum result res;

  if (device->driver)
  {
    res = usbDriverConfigure(device->driver, request, device->state.buffer,
        &length);

    if (res == E_OK)
    {
      sendResponse(device, device->state.buffer, length);
    }
    else
    {
      usbEpSetStalled(device->ep0in, true);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void processStandardRequest(struct UsbDevice *device,
    const struct UsbSetupPacket *packet)
{
  uint16_t length = DATA_BUFFER_SIZE;
  enum result res;

  res = usbHandleStandardRequest(device, packet, device->state.buffer,
      &length);

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(device, device->state.buffer, length);
  }
  else
  {
    usbEpSetStalled(device->ep0in, true);
  }
}
/*----------------------------------------------------------------------------*/
static void resetDevice(struct UsbDevice *device)
{
  usbEpSetEnabled(device->ep0in, true, EP0_BUFFER_SIZE);
  usbEpSetEnabled(device->ep0out, true, EP0_BUFFER_SIZE);
}
/*----------------------------------------------------------------------------*/
static void sendResponse(struct UsbDevice *device, const uint8_t *data,
    uint16_t length)
{
  struct UsbRequest *request;
  uint8_t chunkCount;

  chunkCount = length / EP0_BUFFER_SIZE + 1;
  /* Send zero-length packet to finalize transfer */
  if (length && !(length % EP0_BUFFER_SIZE))
    ++chunkCount;

  if (queueSize(&device->requestPool) < chunkCount)
    return;

  for (uint8_t index = 0; index < chunkCount; ++index)
  {
    queuePop(&device->requestPool, &request);

    const uint16_t chunk = EP0_BUFFER_SIZE < length ? EP0_BUFFER_SIZE : length;

    if (chunk)
      memcpy(request->buffer, data, chunk);
    request->length = chunk;
    request->status = 0;

    data += chunk;
    length -= chunk;

    usbEpEnqueue(device->ep0in, request);
  }
}
/*----------------------------------------------------------------------------*/
static void updateStatus(void *object, uint8_t status)
{
  struct UsbDevice * const device = object;

  if (status & DEVICE_STATUS_RESET)
    resetDevice(device);

  if (device->driver)
    usbDriverUpdateStatus(device->driver, status);
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  const struct UsbDeviceBaseConfig parentConfig = {
      .dm = config->dm,
      .dp = config->dp,
      .connect = config->connect,
      .vbus = config->vbus,
      .channel = config->channel
  };
  struct UsbDevice * const device = object;
  enum result res;

  device->base = init(UsbDeviceBase, &parentConfig);
  if (!device->base)
    return E_ERROR;

  device->currentConfiguration = 0;
  device->driver = 0;

  device->state.buffer = malloc(DATA_BUFFER_SIZE);
  if (!device->state.buffer)
    return E_MEMORY;
  device->state.left = 0;

  /* Create control endpoints */
  device->ep0in = usbDevAllocate(device->base, EP_DIRECTION_IN | EP_ADDRESS(0));
  if (!device->ep0in)
    return E_MEMORY;
  device->ep0out = usbDevAllocate(device->base, EP_ADDRESS(0));
  if (!device->ep0out)
    return E_MEMORY;

  res = queueInit(&device->requestPool, sizeof(struct UsbRequest *),
      CONFIG_USB_CONTROL_REQUESTS * 2);
  if (res != E_OK)
    return res;

  /* Allocate requests */
  device->requests = malloc(2 * CONFIG_USB_CONTROL_REQUESTS
      * sizeof(struct UsbRequest));
  if (!device->requests)
    return E_MEMORY;

  int8_t index;

  for (index = 0; index < 2 * CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    res = usbRequestInit(device->requests + index, EP0_BUFFER_SIZE);
    if (res != E_OK)
      return res;
  }

  for (index = 0; index < CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    struct UsbRequest * const request = device->requests + index;

    usbRequestCallback(request, controlInHandler, device);
    queuePush(&device->requestPool, &request);
  }

  for (; index < 2 * CONFIG_USB_CONTROL_REQUESTS; ++index)
  {
    usbRequestCallback(device->requests + index, controlOutHandler, device);
    usbEpEnqueue(device->ep0out, device->requests + index);
  }

  usbDeviceBaseSetHandler(device->base, updateStatus, device);

  /* Enable interrupts */
  resetDevice(device);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  usbEpClear(device->ep0in);
  usbEpClear(device->ep0out);

  assert(queueSize(&device->requestPool) == 2 * CONFIG_USB_CONTROL_REQUESTS);

  for (int8_t index = 2 * CONFIG_USB_CONTROL_REQUESTS - 1; index >= 0; --index)
    usbRequestDeinit(device->requests + index);
  free(device->requests);

  queueDeinit(&device->requestPool);
  deinit(device->ep0out);
  deinit(device->ep0in);
  free(device->state.buffer);

  deinit(device->base);
}
/*----------------------------------------------------------------------------*/
static void *devAllocate(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;

  return UsbDeviceBase->allocate(device->base, address);
}
/*----------------------------------------------------------------------------*/
static enum result devBind(void *object, void *driver)
{
  //TODO Add spinlock
  struct UsbDevice * const device = object;

  device->driver = driver;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  struct UsbDevice * const device = object;

  UsbDeviceBase->setAddress(device->base, address);
}
/*----------------------------------------------------------------------------*/
static void devSetConfigured(void *object, bool state)
{
  struct UsbDevice * const device = object;

  UsbDeviceBase->setConfigured(device->base, state);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;

  UsbDeviceBase->setConnected(device->base, state);
}
