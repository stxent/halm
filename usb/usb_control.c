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
struct LocalData
{
#ifdef CONFIG_USB_COMPOSITE
  struct List drivers;
  struct CompositeDevice *composite;
#else
  struct UsbDriver *driver;
#endif

  struct UsbRequest requests[REQUEST_POOL_SIZE];

  struct
  {
    struct UsbSetupPacket packet;
    uint16_t left;
    uint8_t buffer[DATA_BUFFER_SIZE];
  } state;
};
/*----------------------------------------------------------------------------*/
static struct LocalData *localDataAllocate(struct UsbControl *);
static void localDataFree(struct LocalData *);
static enum result localDataConfigureDriver(struct LocalData *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
static void localDataUpdateStatus(struct LocalData *, uint8_t);
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
static struct LocalData *localDataAllocate(struct UsbControl *control)
{
  struct LocalData *local = malloc(sizeof(struct LocalData));
  if (!local)
    return 0;

  local->state.left = 0;
  memset(&local->state.packet, 0, sizeof(local->state.packet));

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE; ++index)
  {
    if (usbRequestInit(local->requests + index, EP0_BUFFER_SIZE) != E_OK)
      return 0;
  }

#ifdef CONFIG_USB_COMPOSITE
  if (listInit(&local->drivers, sizeof(const struct UsbDriver *)) != E_OK)
    return 0;

  const struct CompositeDeviceConfig compositeConfig = {
      .control = control
  };

  local->composite = init(CompositeDevice, &compositeConfig);
  if (!local->composite)
    return 0;
#else
  local->driver = 0;
#endif

  return local;
}
/*----------------------------------------------------------------------------*/
static void localDataFree(struct LocalData *local)
{
#ifdef CONFIG_USB_COMPOSITE
  deinit(local->composite);

  assert(listEmpty(&local->drivers));
  listDeinit(&local->drivers);
#else
  assert(local->driver == 0);
#endif

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE; ++index)
    usbRequestDeinit(local->requests + index);

  free(local);
}
/*----------------------------------------------------------------------------*/
static enum result localDataConfigureDriver(struct LocalData *local,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
#ifdef CONFIG_USB_COMPOSITE
  struct ListNode *currentNode = listFirst(&local->drivers);
  struct UsbDriver *current;
  enum result res = E_INVALID;

  while (currentNode)
  {
    listData(&local->drivers, currentNode, &current);
    res = usbDriverConfigure(current, packet, payload, payloadLength,
        response, responseLength, maxResponseLength);
    if (res == E_OK || (res != E_OK && res != E_INVALID))
      break;
    currentNode = listNext(currentNode);
  }

  return res;
#else
  if (!local->driver)
    return E_INVALID;

  return usbDriverConfigure(local->driver, packet, payload, payloadLength,
      response, responseLength, maxResponseLength);
#endif
}
/*----------------------------------------------------------------------------*/
static void localDataUpdateStatus(struct LocalData *local, uint8_t status)
{
#ifdef CONFIG_USB_COMPOSITE
  struct ListNode *currentNode = listFirst(&local->drivers);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&local->drivers, currentNode, &current);
    usbDriverUpdateStatus(current, status);
    currentNode = listNext(currentNode);
  }
#else
  if (local->driver)
    usbDriverUpdateStatus(local->driver, status);
#endif
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
  struct LocalData * const local = control->local;
  struct UsbSetupPacket * const packet = &local->state.packet;

  if (request->status == REQUEST_CANCELLED)
  {
    queuePush(&control->requestPool, &request);
    return;
  }

  uint16_t length = 0;
  enum result res = E_BUSY;

  if (request->status == REQUEST_SETUP)
  {
    local->state.left = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      res = usbHandleStandardRequest(control, packet, local->state.buffer,
          &length, DATA_BUFFER_SIZE);

      if (res != E_OK)
      {
        res = localDataConfigureDriver(control->local, packet, 0, 0,
            local->state.buffer, &length, DATA_BUFFER_SIZE);
      }
    }
    else if (packet->length <= DATA_BUFFER_SIZE)
    {
      local->state.left = packet->length;
    }
  }
  else if (local->state.left && request->length <= local->state.left)
  {
    /* Erroneous packets are ignored */
    memcpy(local->state.buffer + (packet->length - local->state.left),
        request->buffer, request->length);
    local->state.left -= request->length;

    if (!local->state.left)
    {
      res = localDataConfigureDriver(control->local, packet,
          local->state.buffer, packet->length, 0, 0, 0);
    }
  }

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(control, local->state.buffer, length);
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
enum result usbControlAppendDescriptor(struct UsbControl *control,
    const void *descriptor)
{
  return listPush(&control->descriptors, &descriptor);
}
/*----------------------------------------------------------------------------*/
uint8_t usbControlCompositeIndex(const struct UsbControl *control)
{
  const struct ListNode *currentNode = listFirst(&control->descriptors);
  const struct UsbDescriptor *current;
  uint8_t last = 0;

  while (currentNode)
  {
    listData(&control->descriptors, currentNode, &current);
    if (current->descriptorType == DESCRIPTOR_TYPE_INTERFACE)
    {
      struct UsbInterfaceDescriptor * const descriptor =
          (struct UsbInterfaceDescriptor *)current;
      last = descriptor->interfaceNumber + 1;
    }
    currentNode = listNext(currentNode);
  }

  return last;
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

  struct LocalData * const local = control->local;

#ifdef CONFIG_USB_COMPOSITE
  const enum result res = listPush(&local->drivers, &driver);

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

    compositeDeviceUpdate(local->composite, interfaceCount, totalLength);
  }

  return res;
#else
  if (local->driver)
    return E_ERROR;

  local->driver = driver;
  return E_OK;
#endif
}
/*----------------------------------------------------------------------------*/
void usbControlResetDrivers(struct UsbControl *control)
{
  localDataUpdateStatus(control->local, DEVICE_STATUS_RESET);
}
/*----------------------------------------------------------------------------*/
void usbControlUnbindDriver(struct UsbControl *control, const void *driver)
{
  struct LocalData * const local = control->local;

#ifdef CONFIG_USB_COMPOSITE
  struct ListNode *currentNode = listFirst(&local->drivers);
  const struct UsbDriver *current;

  while (currentNode)
  {
    listData(&local->drivers, currentNode, &current);
    if (current == driver)
    {
      listErase(&local->drivers, currentNode);
      return;
    }
    currentNode = listNext(currentNode);
  }
#else
  assert(local->driver == driver);
  local->driver = 0;
#endif
}
/*----------------------------------------------------------------------------*/
void usbControlUpdateStatus(struct UsbControl *control, uint8_t status)
{
  if (status & DEVICE_STATUS_RESET)
    resetDevice(control);

  localDataUpdateStatus(control->local, status & ~DEVICE_STATUS_RESET);
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

  control->local = localDataAllocate(control);
  if (!control->local)
    return E_MEMORY;

  /* Initialize request pool */
  res = queueInit(&control->requestPool, sizeof(struct UsbRequest *),
      REQUEST_POOL_SIZE);
  if (res != E_OK)
    return res;

  /* Enable interrupts */
  resetDevice(control);

  /* Enqueue requests after endpoint enabling */
  struct UsbRequest *request =
      ((struct LocalData *)control->local)->requests;

  for (unsigned short index = 0; index < REQUEST_POOL_SIZE / 2; ++index)
  {
    usbRequestCallback(request, controlInHandler, control);
    queuePush(&control->requestPool, &request);
    ++request;

    usbRequestCallback(request, controlOutHandler, control);
    usbEpEnqueue(control->ep0out, request);
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

  localDataFree(control->local);

  assert(listEmpty(&control->descriptors));
  listDeinit(&control->descriptors);
}
