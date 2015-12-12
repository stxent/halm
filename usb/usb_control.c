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
#define DATA_BUFFER_SIZE  (CONFIG_USB_CONTROL_REQUESTS * EP0_BUFFER_SIZE)
#define REQUEST_POOL_SIZE (CONFIG_USB_CONTROL_REQUESTS * 2)
/*----------------------------------------------------------------------------*/
static enum result configureDrivers(struct UsbControl *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
static void controlInHandler(struct UsbRequest *, void *);
static void controlOutHandler(struct UsbRequest *, void *);
static void resetDevice(struct UsbControl *);
static void sendResponse(struct UsbControl *, const uint8_t *, uint16_t);
static void updateStatus(struct UsbControl *, uint8_t);
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
static enum result configureDrivers(struct UsbControl *control,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct ListNode *currentNode = listFirst(&control->drivers);
  struct UsbDriver *current;
  enum result res = E_INVALID;

  while (currentNode)
  {
    listData(&control->drivers, currentNode, &current);
    res = usbDriverConfigure(current, packet, payload, payloadLength,
        response, responseLength, maxResponseLength);
    if (res == E_OK || (res != E_OK && res != E_INVALID))
      break;
    currentNode = listNext(currentNode);
  }

  return res;
}
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

      if (res != E_OK)
      {
        res = configureDrivers(control, packet, 0, 0, control->state.buffer,
            &length, DATA_BUFFER_SIZE);
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

    if (!control->state.left)
    {
      res = configureDrivers(control, packet, control->state.buffer,
          packet->length, 0, 0, 0);
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
static void updateStatus(struct UsbControl *control, uint8_t status)
{
  struct ListNode *currentNode = listFirst(&control->drivers);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&control->drivers, currentNode, &current);
    usbDriverUpdateStatus(current, status);
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
enum result usbControlAppendDescriptor(struct UsbControl *control,
    const void *descriptor)
{
  return listPush(&control->descriptors, &descriptor);
}
/*----------------------------------------------------------------------------*/
uint8_t usbControlCompositeIndex(const struct UsbControl *control,
    uint8_t configuration)
{
  const struct ListNode *currentNode = listFirst(&control->descriptors);
  const struct UsbDescriptor *current;
  uint8_t currentConfiguration = 0;
  uint8_t count = 0;

  while (currentNode)
  {
    listData(&control->descriptors, currentNode, &current);

    if (current->descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
    {
      const struct UsbConfigurationDescriptor * const descriptor =
          (const struct UsbConfigurationDescriptor *)current;

      currentConfiguration = descriptor->configurationValue;
    }
    else if (current->descriptorType == DESCRIPTOR_TYPE_INTERFACE
        && currentConfiguration == configuration)
    {
      ++count;
    }

    currentNode = listNext(currentNode);
  }

  return count;
}
/*----------------------------------------------------------------------------*/
void usbControlEraseDescriptor(struct UsbControl *control,
    const void *descriptor)
{
  struct ListNode *currentNode = listFirst(&control->descriptors);
  const struct UsbDescriptor *current;

  while (currentNode)
  {
    listData(&control->descriptors, currentNode, &current);
    if (current == descriptor)
    {
      listErase(&control->descriptors, currentNode);
      return;
    }
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
enum result usbControlBindDriver(struct UsbControl *control, void *driver)
{
  if (!driver)
    return E_VALUE;

  const enum result res = listPush(&control->drivers, &driver);

  if (res == E_OK)
  {
    struct ListNode *currentNode = listFirst(&control->descriptors);
    const struct UsbDescriptor *current;
    uint16_t totalLength = 0;
    uint8_t interfaceCount = 0;

    while (currentNode)
    {
      listData(&control->descriptors, currentNode, &current);

      if (current->descriptorType == DESCRIPTOR_TYPE_INTERFACE)
        ++interfaceCount;
      if (current->descriptorType != DESCRIPTOR_TYPE_DEVICE
          && current->descriptorType != DESCRIPTOR_TYPE_STRING)
      {
        totalLength += current->length;
      }

      currentNode = listNext(currentNode);
    }

    compositeDeviceUpdate(control->composite, interfaceCount, totalLength);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbControlResetDrivers(struct UsbControl *control)
{
  updateStatus(control, DEVICE_STATUS_RESET);
}
/*----------------------------------------------------------------------------*/
void usbControlUnbindDriver(struct UsbControl *control, const void *driver)
{
  struct ListNode *currentNode = listFirst(&control->drivers);
  const struct UsbDriver *current;

  while (currentNode)
  {
    listData(&control->drivers, currentNode, &current);
    if (current == driver)
    {
      listErase(&control->drivers, currentNode);
      return;
    }
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
void usbControlUpdateStatus(struct UsbControl *control, uint8_t status)
{
  if (status & DEVICE_STATUS_RESET)
    resetDevice(control);

  updateStatus(control, status & ~DEVICE_STATUS_RESET);
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

  control->state.buffer = malloc(DATA_BUFFER_SIZE);
  if (!control->state.buffer)
    return E_MEMORY;
  control->state.left = 0;

  res = listInit(&control->descriptors, sizeof(const struct UsbDescriptor *));
  if (res != E_OK)
    return res;
  res = listInit(&control->drivers, sizeof(const struct UsbDriver *));
  if (res != E_OK)
    return res;

  /* Create control endpoints */
  control->ep0in = usbDevAllocate(control->owner, EP_DIRECTION_IN
      | EP_ADDRESS(0));
  if (!control->ep0in)
    return E_MEMORY;
  control->ep0out = usbDevAllocate(control->owner, EP_ADDRESS(0));
  if (!control->ep0out)
    return E_MEMORY;

  res = queueInit(&control->requestPool, sizeof(struct UsbRequest *),
      REQUEST_POOL_SIZE);
  if (res != E_OK)
    return res;

  /* Allocate requests */
  control->requests = malloc(REQUEST_POOL_SIZE * sizeof(struct UsbRequest));
  if (!control->requests)
    return E_MEMORY;

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE; ++index)
  {
    res = usbRequestInit(control->requests + index, EP0_BUFFER_SIZE);
    if (res != E_OK)
      return res;
  }

  /* Enable interrupts */
  resetDevice(control);

  /* Enqueue requests after endpoint enabling */
  struct UsbRequest *request = control->requests;

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE / 2; ++index)
  {
    usbRequestCallback(request, controlInHandler, control);
    queuePush(&control->requestPool, &request);
    ++request;

    usbRequestCallback(request, controlOutHandler, control);
    usbEpEnqueue(control->ep0out, request);
    ++request;
  }

  const struct CompositeDeviceConfig compositeConfig = {
      .control = control
  };

  control->composite = init(CompositeDevice, &compositeConfig);
  if (!control->composite)
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void controlDeinit(void *object)
{
  struct UsbControl * const control = object;

  usbEpClear(control->ep0in);
  usbEpClear(control->ep0out);

  assert(queueSize(&control->requestPool) == REQUEST_POOL_SIZE);

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE; ++index)
    usbRequestDeinit(control->requests + index);
  free(control->requests);

  queueDeinit(&control->requestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);

  assert(listEmpty(&control->descriptors));
  listDeinit(&control->descriptors);
  assert(listEmpty(&control->drivers));
  listDeinit(&control->drivers);

  free(control->state.buffer);
}
