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
#define EP0_BUFFER_SIZE 64
#define REQUEST_COUNT   4
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static enum result devBind(void *, void *);
static void *devAllocate(void *, uint16_t, uint8_t);
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
static void controlInHandler(struct UsbRequest *, void *);
static void controlOutHandler(struct UsbRequest *, void *);
static void processRequest(struct UsbDevice *, struct UsbRequest *);
static void processStandardRequest(struct UsbDevice *, struct UsbSetupPacket *);
static void sendResponse(struct UsbDevice *, const uint8_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static void controlInHandler(struct UsbRequest *request, void *argument)
{
  struct UsbDevice * const device = argument;

  queuePush(&device->responseQueue, &request);
}
/*----------------------------------------------------------------------------*/
bool _HandleRequest(struct UsbSetupPacket *pSetup, int *piLen, uint8_t **ppbData); //FIXME
/*----------------------------------------------------------------------------*/
static void controlOutHandler(struct UsbRequest *request,
    void *argument)
{
  struct UsbDevice * const device = argument;
  struct UsbSetupPacket * const packet = &device->setupPacket;

  if (request->status & EP_STATUS_SETUP)
  {
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_STANDARD)
    {
      device->left = 0;
      processRequest(device, request);
    }
    else
    {
      const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

      device->left = packet->length;

      if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
        processStandardRequest(device, packet);
    }
  }
  else
  {
    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_STANDARD)
    {
      processRequest(device, request);
    }
    else if (device->left)
    {
      memcpy(device->buffer + (packet->length - device->left), request->buffer,
          request->length);
      device->left -= request->length;

      if (!device->left)
        processStandardRequest(device, packet);
    }
  }

  usbEpEnqueue(device->ep0out, request);
}
/*----------------------------------------------------------------------------*/
static void processRequest(struct UsbDevice *device, struct UsbRequest *request)
{
  uint16_t length = EP0_BUFFER_SIZE; //FIXME Length of the buffer
  enum result res;

  if (device->driver)
  {
    res = usbDriverConfigure(device->driver, request, device->buffer, &length);

    if (res == E_OK)
    {
      sendResponse(device, device->buffer, length);
    }
    else
    {
      usbEpSetStalled(device->ep0in, true);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void processStandardRequest(struct UsbDevice *device,
    struct UsbSetupPacket *packet)
{
  int ilen = packet->length; //FIXME
  uint8_t *bubuData = device->buffer;

  // ask installed handler to process request
  if (_HandleRequest(packet, &ilen, &bubuData))
  {
    // send smallest of requested and offered length
    int lllen = ilen < packet->length ? ilen : packet->length;
    // send first part (possibly a zero-length status message)
    sendResponse(device, bubuData, lllen);
  }
  else
  {
    usbEpSetStalled(device->ep0in, true);
  }
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

  if (queueSize(&device->responseQueue) < chunkCount)
    return;

  for (uint8_t index = 0; index < chunkCount; ++index)
  {
    uint16_t chunk;

    queuePop(&device->responseQueue, &request);

    chunk = EP0_BUFFER_SIZE < length ? EP0_BUFFER_SIZE : length;
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
static enum result devInit(void *object, const void *configBase)
{
  const struct UsbDeviceConfig * const config = configBase;
  const struct UsbDeviceBaseConfig parentConfig = {
      .dm = config->dm,
      .dp = config->dp,
      .connect = config->connect,
      .channel = config->channel
  };
  struct UsbDevice * const device = object;
  enum result res;

  device->device = init(UsbDeviceBase, &parentConfig);
  if (!device->device)
    return E_ERROR;

  device->driver = 0;
  device->left = 0;

  device->buffer = malloc(EP0_BUFFER_SIZE); //FIXME Select size
  if (!device->buffer)
    return E_MEMORY;

  //FIXME Remove magic
  device->ep0in = usbDevAllocate(device->device, EP0_BUFFER_SIZE, 0x80);
  if (!device->ep0in)
    return E_MEMORY;
  device->ep0out = usbDevAllocate(device->device, EP0_BUFFER_SIZE, 0x00);
  if (!device->ep0out)
    return E_MEMORY;

  res = queueInit(&device->responseQueue, sizeof(struct UsbRequest *),
      REQUEST_COUNT);
  if (res != E_OK)
    return res;

  //TODO Rewrite allocation, reduce memory consumption
  for (unsigned int index = 0; index < REQUEST_COUNT; ++index)
  {
    struct UsbRequest *request = malloc(sizeof(struct UsbRequest));
    usbRequestInit(request, EP0_BUFFER_SIZE);

    //TODO Add function for callback setup
    request->callback = controlInHandler;
    request->callbackArgument = device;

    queuePush(&device->responseQueue, &request);
  }

  for (unsigned int index = 0; index < REQUEST_COUNT; ++index)
  {
    struct UsbRequest *request = malloc(sizeof(struct UsbRequest));
    usbRequestInit(request, EP0_BUFFER_SIZE);

    request->callback = controlOutHandler;
    request->callbackArgument = device;

    usbEpEnqueue(device->ep0out, request);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct UsbDevice * const device = object;

  //TODO
  free(device->buffer);

  deinit(device->device);
}
/*----------------------------------------------------------------------------*/
static void *devAllocate(void *object, uint16_t size, uint8_t address)
{
  struct UsbDevice * const device = object;

  return UsbDeviceBase->allocate(device->device, size, address);
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

  UsbDeviceBase->setAddress(device->device, address);
}
/*----------------------------------------------------------------------------*/
static void devSetConfigured(void *object, bool state)
{
  struct UsbDevice * const device = object;

  UsbDeviceBase->setConfigured(device->device, state);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct UsbDevice * const device = object;

  UsbDeviceBase->setConnected(device->device, state);
}
