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
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_string.h>
#include <halm/usb/usb_trace.h>
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

  struct UsbSetupPacket packet;
  uint16_t left;
  uint8_t data[DATA_BUFFER_SIZE];
};
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(struct UsbControl *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
static void fillConfigurationDescriptor(const struct UsbControl *, void *);
static void fillDeviceDescriptor(const struct UsbControl *, void *);
static enum result handleDeviceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum result handleEndpointRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleInterfaceRequest(const struct UsbSetupPacket *,
    uint8_t *, uint16_t *);
static enum result handleStringRequest(struct UsbControl *, uint16_t, uint16_t,
    uint8_t *, uint16_t *, uint16_t);
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
static enum result driverConfigure(struct UsbControl *control,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  enum result res = usbDriverConfigure(control->driver, packet,
      payload, payloadLength, response, responseLength, maxResponseLength);

  if (res == E_INVALID && type == REQUEST_TYPE_STANDARD)
  {
    switch (recipient)
    {
      case REQUEST_RECIPIENT_DEVICE:
        res = handleDeviceRequest(control, packet, response, responseLength,
            maxResponseLength);
        break;

      case REQUEST_RECIPIENT_INTERFACE:
        if (packet->index <= usbDevGetInterface(control->owner))
          res = handleInterfaceRequest(packet, response, responseLength);
        break;

      case REQUEST_RECIPIENT_ENDPOINT:
        res = handleEndpointRequest(control, packet, response, responseLength);
        break;
    }
  }

  if (res != E_OK)
    return res;

  if (type == REQUEST_TYPE_STANDARD && recipient == REQUEST_RECIPIENT_DEVICE
      && packet->request == REQUEST_GET_DESCRIPTOR)
  {
    /* Post-process device and configuration descriptors */
    switch (DESCRIPTOR_TYPE(packet->value))
    {
      case DESCRIPTOR_TYPE_DEVICE:
        fillDeviceDescriptor(control, response);
        break;

      case DESCRIPTOR_TYPE_CONFIGURATION:
        fillConfigurationDescriptor(control, response);
        break;
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void fillConfigurationDescriptor(const struct UsbControl *control,
    void *buffer)
{
  struct UsbConfigurationDescriptor * const descriptor = buffer;

  descriptor->attributes = CONFIGURATION_DESCRIPTOR_DEFAULT;
  if (!control->current)
    descriptor->attributes |= CONFIGURATION_DESCRIPTOR_SELF_POWERED;
  if (control->rwu)
    descriptor->attributes |= CONFIGURATION_DESCRIPTOR_REMOTE_WAKEUP;
  descriptor->maxPower = ((control->current + 1) >> 1);
}
/*----------------------------------------------------------------------------*/
static void fillDeviceDescriptor(const struct UsbControl *control, void *buffer)
{
  struct UsbDeviceDescriptor * const descriptor = buffer;

  descriptor->idVendor = toLittleEndian16(control->vid);
  descriptor->idProduct = toLittleEndian16(control->pid);

  const struct ListNode *current = listFirst(&control->strings);
  struct UsbString entry;
  size_t index = 0;

  while (current)
  {
    listData(&control->strings, current, &entry);

    switch ((enum usbStringType)entry.type)
    {
      case USB_STRING_VENDOR:
        descriptor->manufacturer = index;
        break;

      case USB_STRING_PRODUCT:
        descriptor->product = index;
        break;

      case USB_STRING_SERIAL:
        descriptor->serialNumber = index;
        break;

      default:
        break;
    }

    current = listNext(current);
    ++index;
  }
}
/*----------------------------------------------------------------------------*/
static enum result handleDeviceRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
    {
      uint8_t status = 0;

      if (!control->current)
        status |= STATUS_SELF_POWERED;
      if (control->rwu)
        status |= STATUS_REMOTE_WAKEUP;

      response[0] = status;
      response[1] = 0;
      *responseLength = 2;
      return E_OK;
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
    {
      const bool set = packet->request == REQUEST_SET_FEATURE;

      usbTrace("control: %s device feature %u",
          set ? "set" : "clear", packet->value);

      if (packet->value == FEATURE_REMOTE_WAKEUP)
      {
        control->rwu = set;
        *responseLength = 0;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    case REQUEST_SET_ADDRESS:
      usbTrace("control: set address %u", packet->value);

      usbDevSetAddress(control->owner, packet->value);
      *responseLength = 0;
      return E_OK;

    case REQUEST_GET_DESCRIPTOR:
    {
      enum result res;

      usbTrace("control: get descriptor %u:%u, length %u",
          DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
          packet->length);

      res = usbExtractDescriptorData(control->driver, packet->value,
          response, responseLength, maxResponseLength);

      if (res == E_INVALID)
      {
        res = handleStringRequest(control, packet->value, packet->index,
            response, responseLength, maxResponseLength);
      }

      return res;
    }

    case REQUEST_GET_CONFIGURATION:
      response[0] = 1;
      *responseLength = 1;
      return E_OK;

    case REQUEST_SET_CONFIGURATION:
      usbTrace("control: set configuration %u", packet->value);

      if (packet->value == 1)
      {
        usbDriverEvent(control->driver, USB_DEVICE_EVENT_RESET);
        *responseLength = 0;
        return E_OK;
      }
      else
        return E_VALUE;

    default:
      usbTrace("control: unsupported device request 0x%02X", packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result handleEndpointRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength)
{
  struct UsbEndpoint * const endpoint = usbDevCreateEndpoint(control->owner,
      packet->index);

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Is endpoint halted or not */
      response[0] = (uint8_t)usbEpIsStalled(endpoint);
      response[1] = 0; /* Must be set to zero */
      *responseLength = 2;
      return E_OK;

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
    {
      const bool set = packet->request == REQUEST_SET_FEATURE;

      usbTrace("control: %s endpoint %0x02X feature %u",
          set ? "set" : "clear", packet->index, packet->value);

      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        usbEpSetStalled(endpoint, set);
        *responseLength = 0;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      usbTrace("control: unsupported request 0x%02X to endpoint 0x%02X",
          packet->request, packet->index);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result handleInterfaceRequest(const struct UsbSetupPacket *packet,
    uint8_t *response, uint16_t *responseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Two bytes are reserved for future use and must be set to zero */
      response[0] = response[1] = 0;
      *responseLength = 2;
      return E_OK;

    case REQUEST_GET_INTERFACE:
      response[0] = 0;
      *responseLength = 1;
      return E_OK;

    case REQUEST_SET_INTERFACE:
      usbTrace("control: set interface %u", packet->value);

      /* Only one interface is supported by default */
      if (packet->value == 0)
      {
        *responseLength = 0;
        return E_OK;
      }
      else
        return E_VALUE;

    default:
      usbTrace("control: unsupported interface request 0x%02X",
          packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result handleStringRequest(struct UsbControl *control,
    uint16_t keyword, uint16_t langid, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);

  if (descriptorType != DESCRIPTOR_TYPE_STRING)
    return E_INVALID;

  const struct ListNode *current = listFirst(&control->strings);
  size_t index = 0;

  while (current)
  {
    struct UsbString entry;
    listData(&control->strings, current, &entry);

    if (index == descriptorIndex)
    {
      struct UsbDescriptor * const header = (struct UsbDescriptor *)response;

      assert((entry.functor(entry.argument, langid, header, 0),
          header->length <= maxResponseLength));
      (void)maxResponseLength;

      entry.functor(entry.argument, langid, header, response);
      *responseLength = header->length;
      return E_OK;
    }

    current = listNext(current);
    ++index;
  }

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result privateDataAllocate(struct UsbControl *control)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));

  if (!privateData)
    return E_MEMORY;
  control->privateData = privateData;

  privateData->left = 0;
  memset(&privateData->packet, 0, sizeof(privateData->packet));

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
  struct UsbSetupPacket * const packet = &privateData->packet;

  assert(control->driver);

  if (status == USB_REQUEST_CANCELLED)
    return;

  uint16_t length = 0;
  enum result res = E_BUSY;

  if (status == USB_REQUEST_SETUP)
  {
    privateData->left = 0;
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      res = driverConfigure(control, packet, 0, 0, privateData->data,
          &length, DATA_BUFFER_SIZE);
    }
    else if (packet->length <= DATA_BUFFER_SIZE)
    {
      privateData->left = packet->length;
    }
  }
  else if (privateData->left && request->length <= privateData->left)
  {
    /* Erroneous packets are ignored */
    memcpy(privateData->data + (packet->length - privateData->left),
        request->buffer, request->length);
    privateData->left -= request->length;

    if (!privateData->left)
    {
      res = driverConfigure(control, packet, privateData->data,
          packet->length, 0, 0, 0);
    }
  }

  if (res == E_OK)
  {
    /* Send smallest of requested and offered lengths */
    length = length < packet->length ? length : packet->length;
    sendResponse(control, privateData->data, length);
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
  unsigned int chunkCount;

  chunkCount = (length + (EP0_BUFFER_SIZE - 1)) / EP0_BUFFER_SIZE;
  /* Send zero-length packet to finalize transfer */
  if (length % EP0_BUFFER_SIZE == 0)
    ++chunkCount;

  if (queueSize(&control->inRequestPool) < chunkCount)
    return;

  for (unsigned int index = 0; index < chunkCount; ++index)
  {
    struct UsbRequest *request;
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
enum result usbControlBindDriver(struct UsbControl *control, void *driver)
{
  assert(driver);
  assert(!control->driver);

  control->driver = driver;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbControlUnbindDriver(struct UsbControl *control)
{
  control->driver = 0;
}
/*----------------------------------------------------------------------------*/
void usbControlEvent(struct UsbControl *control, unsigned int event)
{
  assert(control->driver);

  if (event == USB_DEVICE_EVENT_RESET)
    resetDevice(control);
  else
    usbDriverEvent(control->driver, event);
}
/*----------------------------------------------------------------------------*/
void usbControlSetPower(struct UsbControl *control, uint16_t current)
{
  control->current = current;
}
/*----------------------------------------------------------------------------*/
enum result usbControlStringAppend(struct UsbControl *control,
    struct UsbString string)
{
  if (!listFind(&control->strings, &string))
  {
    if (string.type == USB_STRING_HEADER)
      assert(listEmpty(&control->strings));
    else
      assert(!listEmpty(&control->strings));

    return listPush(&control->strings, &string);
  }
  else
    return E_EXIST;
}
/*----------------------------------------------------------------------------*/
void usbControlStringErase(struct UsbControl *control, struct UsbString string)
{
  struct ListNode * const node = listFind(&control->strings, &string);

  if (node)
    listErase(&control->strings, node);
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

  res = privateDataAllocate(control);
  if (res != E_OK)
    return E_MEMORY;

  control->driver = 0;
  control->current = 0;
  control->vid = config->vid;
  control->pid = config->pid;
  control->rwu = false;

  /* Initialize request queue */
  res = queueInit(&control->inRequestPool, sizeof(struct UsbRequest *),
      REQUEST_POOL_SIZE);
  if (res != E_OK)
    return res;

  /* Initialize string list */
  res = listInit(&control->strings, sizeof(struct UsbString));
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

  listDeinit(&control->strings);
  queueDeinit(&control->inRequestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);

  privateDataFree(control);
}
