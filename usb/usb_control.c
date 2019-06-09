/*
 * usb_control.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/containers/tg_list.h>
#include <xcore/memory.h>
#include <halm/generic/pointer_array.h>
#include <halm/usb/usb.h>
#include <halm/usb/usb_control.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define EP0_BUFFER_SIZE   64
#define DATA_BUFFER_SIZE  (CONFIG_USB_DEVICE_CONTROL_REQUESTS * EP0_BUFFER_SIZE)
#define REQUEST_POOL_SIZE (CONFIG_USB_DEVICE_CONTROL_REQUESTS)
/*----------------------------------------------------------------------------*/
DEFINE_LIST(struct UsbString, String, string)
/*----------------------------------------------------------------------------*/
struct ControlUsbRequest
{
  struct UsbRequest base;
  uint8_t payload[EP0_BUFFER_SIZE];
};

struct UsbControl
{
  struct Entity base;

  /* Parent object */
  struct UsbDevice *owner;

  /* Device driver that is currently active */
  struct UsbDriver *driver;

  /* Control endpoints */
  struct UsbEndpoint *ep0in;
  struct UsbEndpoint *ep0out;

  /* Pool for IN requests */
  PointerArray inRequestPool;
  /* Single OUT request */
  struct UsbRequest *outRequest;

  /* List of descriptor strings */
  StringList strings;

  /* Maximum current drawn by the device in mA */
  uint16_t current;
  /* Vendor Identifier */
  uint16_t vid;
  /* Product Identifier */
  uint16_t pid;
  /* Remote wake-up flag */
  bool rwu;

  struct
  {
    size_t left;

    /* Setup packet header */
    struct UsbSetupPacket packet;
    /* Packet payload */
    uint8_t payload[DATA_BUFFER_SIZE];
  } context;

  struct ControlUsbRequest requests[REQUEST_POOL_SIZE + 1];
};
/*----------------------------------------------------------------------------*/
static enum Result driverControl(struct UsbControl *,
    const struct UsbSetupPacket *, void *, uint16_t *, uint16_t);
static void fillConfigurationDescriptor(const struct UsbControl *, void *);
static void fillDeviceDescriptor(struct UsbControl *, void *);
static enum Result handleDescriptorRequest(struct UsbControl *,
    const struct UsbSetupPacket *, void *, uint16_t *, uint16_t);
static enum Result handleDeviceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum Result handleEndpointRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum Result handleInterfaceRequest(const struct UsbSetupPacket *,
    uint8_t *, uint16_t *);
static enum Result handleStringRequest(struct UsbControl *, uint16_t, uint16_t,
    void *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void controlOutHandler(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void enableEndpoints(struct UsbControl *);
static void copySetupPacket(struct UsbSetupPacket *,
    const struct UsbRequest *);
static void resetDevice(struct UsbControl *);
static void sendResponse(struct UsbControl *, const uint8_t *, uint16_t);
static bool usbStringComparator(const void *a, void *b);
/*----------------------------------------------------------------------------*/
static enum Result controlInit(void *, const void *);
static void controlDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbControl = &(const struct EntityClass){
    .size = sizeof(struct UsbControl),
    .init = controlInit,
    .deinit = controlDeinit
};
/*----------------------------------------------------------------------------*/
static enum Result driverControl(struct UsbControl *control,
    const struct UsbSetupPacket *packet, void *buffer, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  enum Result res = usbDriverControl(control->driver, packet, buffer,
      responseLength, maxResponseLength);

  if (res == E_INVALID && type == REQUEST_TYPE_STANDARD)
  {
    switch (recipient)
    {
      case REQUEST_RECIPIENT_DEVICE:
        res = handleDeviceRequest(control, packet, buffer, responseLength,
            maxResponseLength);
        break;

      case REQUEST_RECIPIENT_INTERFACE:
        if (packet->index <= usbDevGetInterface(control->owner))
          res = handleInterfaceRequest(packet, buffer, responseLength);
        break;

      case REQUEST_RECIPIENT_ENDPOINT:
        res = handleEndpointRequest(control, packet, buffer, responseLength);
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
        fillDeviceDescriptor(control, buffer);
        break;

      case DESCRIPTOR_TYPE_CONFIGURATION:
        fillConfigurationDescriptor(control, buffer);
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
static void fillDeviceDescriptor(struct UsbControl *control, void *buffer)
{
  struct UsbDeviceDescriptor * const descriptor = buffer;

  descriptor->idVendor = toLittleEndian16(control->vid);
  descriptor->idProduct = toLittleEndian16(control->pid);

  StringListNode *current = stringListFront(&control->strings);
  size_t index = 0;

  while (current)
  {
    const struct UsbString * const entry = stringListData(current);

    switch ((enum UsbStringType)entry->type)
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

    current = stringListNext(current);
    ++index;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleDescriptorRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, void *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum Result res = usbExtractDescriptorData(control->driver, packet->value,
      response, responseLength, maxResponseLength);

  if (res == E_INVALID)
  {
    res = handleStringRequest(control, packet->value, packet->index,
        response, responseLength, maxResponseLength);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleDeviceRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum Result res = E_OK;

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
    {
      uint8_t flags = 0;

      if (!control->current)
        flags |= STATUS_SELF_POWERED;
      if (control->rwu)
        flags |= STATUS_REMOTE_WAKEUP;

      response[0] = 0;
      response[1] = flags;
      *responseLength = 2;
      break;
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
    {
      const bool set = packet->request == REQUEST_SET_FEATURE;

      usbTrace("control: %s device feature %u",
          set ? "set" : "clear", packet->value);

      if (packet->value == FEATURE_REMOTE_WAKEUP)
        control->rwu = set;
      else
        res = E_VALUE;
      break;
    }

    case REQUEST_SET_ADDRESS:
    {
      usbTrace("control: set address %u", packet->value);

      usbDevSetAddress(control->owner, packet->value);
      break;
    }

    case REQUEST_GET_DESCRIPTOR:
    {
      usbTrace("control: get descriptor %u:%u, length %u",
          DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
          packet->length);

      res = handleDescriptorRequest(control, packet, response, responseLength,
          maxResponseLength);
      break;
    }

    case REQUEST_GET_CONFIGURATION:
    {
      response[0] = DEFAULT_DEVICE_CONFIGURATION;
      *responseLength = 1;
      break;
    }

    case REQUEST_SET_CONFIGURATION:
    {
      usbTrace("control: set configuration %u", packet->value);

      if (packet->value == DEFAULT_DEVICE_CONFIGURATION)
        usbDriverNotify(control->driver, USB_DEVICE_EVENT_RESET);
      else
        res = E_VALUE;
      break;
    }

    default:
    {
      usbTrace("control: unsupported device request 0x%02X", packet->request);
      res = E_INVALID;
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleEndpointRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength)
{
  struct UsbEndpoint * const endpoint = usbDevCreateEndpoint(control->owner,
      packet->index);
  enum Result res = E_OK;

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
    {
      /* Is endpoint halted or not */
      response[0] = usbEpIsStalled(endpoint);
      response[1] = 0; /* Must be set to zero */
      *responseLength = 2;
      break;
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
    {
      const bool set = packet->request == REQUEST_SET_FEATURE;

      usbTrace("control: %s endpoint 0x%02X feature %u",
          set ? "set" : "clear", packet->index, packet->value);

      if (packet->value == FEATURE_ENDPOINT_HALT)
        usbEpSetStalled(endpoint, set);
      else
        res = E_VALUE;
      break;
    }

    default:
    {
      usbTrace("control: unsupported request 0x%02X to endpoint 0x%02X",
          packet->request, packet->index);
      res = E_INVALID;
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleInterfaceRequest(const struct UsbSetupPacket *packet,
    uint8_t *response, uint16_t *responseLength)
{
  enum Result res = E_OK;

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Two bytes are reserved for future use and must be set to zero */
      response[0] = response[1] = 0;
      *responseLength = 2;
      break;

    case REQUEST_GET_INTERFACE:
      response[0] = 0;
      *responseLength = 1;
      break;

    case REQUEST_SET_INTERFACE:
      usbTrace("control: set interface %u", packet->value);

      /* Only one interface is supported by default */
      if (packet->value != 0)
        res = E_VALUE;
      break;

    default:
      usbTrace("control: unsupported interface request 0x%02X",
          packet->request);
      res = E_INVALID;
      break;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleStringRequest(struct UsbControl *control,
    uint16_t keyword, uint16_t langid, void *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);

  if (descriptorType != DESCRIPTOR_TYPE_STRING)
    return E_INVALID;

  StringListNode *current = stringListFront(&control->strings);
  size_t index = 0;

  while (current)
  {
    const struct UsbString * const entry = stringListData(current);

    if (index == descriptorIndex)
    {
      struct UsbDescriptor * const header = response;

      assert((entry->functor(entry->argument, langid, header, 0),
          header->length <= maxResponseLength));
      (void)maxResponseLength;

      entry->functor(entry->argument, langid, header, response);
      *responseLength = header->length;
      return E_OK;
    }

    current = stringListNext(current);
    ++index;
  }

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static void controlInHandler(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status __attribute__((unused)))
{
  struct UsbControl * const control = argument;
  pointerArrayPushBack(&control->inRequestPool, request);
}
/*----------------------------------------------------------------------------*/
static void controlOutHandler(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct UsbControl * const control = argument;
  struct UsbSetupPacket * const packet = &control->context.packet;
  bool ready = false;

  assert(control->driver);

  if (status == USB_REQUEST_CANCELLED)
    return;

  if (status == USB_REQUEST_SETUP)
  {
    copySetupPacket(packet, request);
    control->context.left = 0;

    if (!packet->length || REQUEST_DIRECTION_VALUE(packet->requestType) ==
        REQUEST_DIRECTION_TO_HOST)
    {
      ready = true;
    }
    else if (packet->length <= sizeof(control->context.payload))
    {
      control->context.left = packet->length;
    }
  }
  else if (control->context.left && request->length <= control->context.left)
  {
    /* Copy next chunk of data into the temporary buffer */
    const uint16_t offset = packet->length - control->context.left;

    memcpy(control->context.payload + offset, request->buffer, request->length);
    control->context.left -= request->length;

    if (!control->context.left)
      ready = true;
  }

  if (ready)
  {
    const uint8_t out = REQUEST_DIRECTION_VALUE(packet->requestType) ==
        REQUEST_DIRECTION_TO_DEVICE;
    uint16_t length = 0;

    const enum Result res = driverControl(
        control,
        packet,
        control->context.payload,
        (out ? 0 : &length),
        (out ? 0 : sizeof(control->context.payload))
    );

    if (res == E_OK)
    {
      /* Truncate the response if its length is greater than requested */
      if (packet->length < length)
        length = packet->length;

      sendResponse(control, control->context.payload, length);
    }
    else
    {
      usbEpSetStalled(control->ep0in, true);
    }
  }

  usbEpEnqueue(control->ep0out, request);
}
/*----------------------------------------------------------------------------*/
static void copySetupPacket(struct UsbSetupPacket *packet,
    const struct UsbRequest *request)
{
  const struct UsbSetupPacket * const received =
      (const struct UsbSetupPacket *)request->buffer;

  packet->requestType = received->requestType;
  packet->request = received->request;
  packet->value = fromLittleEndian16(received->value);
  packet->index = fromLittleEndian16(received->index);
  packet->length = fromLittleEndian16(received->length);
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
  size_t chunkCount = (length + (EP0_BUFFER_SIZE - 1)) / EP0_BUFFER_SIZE;

  /* Send zero-length packet to finalize transfer */
  if (length % EP0_BUFFER_SIZE == 0)
    ++chunkCount;

  if (pointerArraySize(&control->inRequestPool) < chunkCount)
    return;

  for (size_t index = 0; index < chunkCount; ++index)
  {
    struct UsbRequest * const request =
        pointerArrayBack(&control->inRequestPool);
    pointerArrayPopBack(&control->inRequestPool);

    const size_t chunk = MIN(length, EP0_BUFFER_SIZE);

    if (chunk)
      memcpy(request->buffer, data, chunk);
    request->length = (uint16_t)chunk;

    data += chunk;
    length -= chunk;

    usbEpEnqueue(control->ep0in, request);
  }
}
/*----------------------------------------------------------------------------*/
static bool usbStringComparator(const void *a, void *b)
{
  const struct UsbString * const aValue = a;
  const struct UsbString * const bValue = b;

  return aValue->functor == bValue->functor
      && aValue->argument == bValue->argument
      && aValue->type == bValue->type;
}
/*----------------------------------------------------------------------------*/
enum Result usbControlBindDriver(struct UsbControl *control, void *driver)
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
void usbControlNotify(struct UsbControl *control, unsigned int event)
{
  assert(control->driver);

  if (event == USB_DEVICE_EVENT_RESET)
    resetDevice(control);
  else
    usbDriverNotify(control->driver, event);
}
/*----------------------------------------------------------------------------*/
void usbControlSetPower(struct UsbControl *control, uint16_t current)
{
  control->current = current;
}
/*----------------------------------------------------------------------------*/
UsbStringNumber usbControlStringAppend(struct UsbControl *control,
    struct UsbString string)
{
  assert(!stringListFind(&control->strings, string));

  const size_t count = stringListSize(&control->strings);

  /* String header must be added first and it must be unique */
  assert((count == 0) == (string.type == USB_STRING_HEADER));

  /* Append a new string to the list */
  return stringListPushBack(&control->strings, string) ?
      (UsbStringNumber)count : -1;
}
/*----------------------------------------------------------------------------*/
void usbControlStringErase(struct UsbControl *control, struct UsbString string)
{
  stringListEraseIf(&control->strings, &string, usbStringComparator);
}
/*----------------------------------------------------------------------------*/
static enum Result controlInit(void *object, const void *configBase)
{
  const struct UsbControlConfig * const config = configBase;
  struct UsbControl * const control = object;

  if (!config->parent)
    return E_VALUE;

  control->owner = config->parent;

  /* Create control endpoints */
  control->ep0in = usbDevCreateEndpoint(control->owner,
      USB_EP_DIRECTION_IN | USB_EP_ADDRESS(0));
  if (!control->ep0in)
    return E_MEMORY;
  control->ep0out = usbDevCreateEndpoint(control->owner, USB_EP_ADDRESS(0));
  if (!control->ep0out)
    return E_MEMORY;

  control->driver = 0;
  control->current = 0;
  control->vid = config->vid;
  control->pid = config->pid;
  control->rwu = false;

  control->context.left = 0;
  memset(&control->context.packet, 0, sizeof(control->context.packet));

  /* Initialize request queue */
  if (!pointerArrayInit(&control->inRequestPool, REQUEST_POOL_SIZE))
    return E_MEMORY;

  /* Initialize list of device strings */
  stringListInit(&control->strings);

  /* Enable endpoints before adding requests in the queues */
  enableEndpoints(control);

  /* Initialize requests */
  struct ControlUsbRequest *request = control->requests;

  for (size_t index = 0; index < REQUEST_POOL_SIZE; ++index)
  {
    usbRequestInit((struct UsbRequest *)request, request->payload,
        sizeof(request->payload), controlInHandler, control);
    pointerArrayPushBack(&control->inRequestPool, request);
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

  assert(pointerArraySize(&control->inRequestPool) == REQUEST_POOL_SIZE);

  stringListDeinit(&control->strings);
  pointerArrayDeinit(&control->inRequestPool);
  deinit(control->ep0out);
  deinit(control->ep0in);
}
