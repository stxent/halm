/*
 * hid_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
#include <xcore/memory.h>
#include <halm/usb/hid.h>
#include <halm/usb/hid_base.h>
#include <halm/usb/hid_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
struct SingleHidDescriptor
{
  struct HidDescriptorBase base;
  struct HidDescriptorEntry entries[1];
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void interfaceDescriptor(const void *, struct UsbDescriptor *, void *);
static void hidDescriptor(const void *, struct UsbDescriptor *, void *);
static void endpointDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct HidBase *,
    const struct UsbSetupPacket *, const void *, uint16_t,
    void *, uint16_t *, uint16_t);
static enum Result handleDeviceRequest(struct HidBase *,
    const struct UsbSetupPacket *, void *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverConfigure(void *, const struct UsbSetupPacket *,
    const void *, uint16_t, void *, uint16_t *, uint16_t);
static const usbDescriptorFunctor *driverDescribe(const void *);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct HidBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .describe = driverDescribe,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const HidBase = &driverTable;
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor deviceDescriptorTable[] = {
    deviceDescriptor,
    configDescriptor,
    interfaceDescriptor,
    hidDescriptor,
    endpointDescriptor,
    0
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload)
  {
    struct UsbDeviceDescriptor * const descriptor = payload;

    descriptor->usb = TO_LITTLE_ENDIAN_16(0x0200);
    descriptor->deviceClass = USB_CLASS_PER_INTERFACE;
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(HID_CONTROL_EP_SIZE);
    descriptor->device = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->numConfigurations = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload)
  {
    struct UsbConfigurationDescriptor * const descriptor = payload;

    descriptor->totalLength = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct SingleHidDescriptor)
        + sizeof(struct UsbEndpointDescriptor));
    descriptor->numInterfaces = 1;
    descriptor->configurationValue = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    struct UsbInterfaceDescriptor * const descriptor = payload;

    descriptor->interfaceNumber = driver->interfaceIndex;
    descriptor->numEndpoints = 1;
    descriptor->interfaceClass = USB_CLASS_HID;
    descriptor->interfaceSubClass = HID_SUBCLASS_NONE;
    descriptor->interfaceProtocol = HID_PROTOCOL_NONE;
  }
}
/*----------------------------------------------------------------------------*/
static void hidDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct SingleHidDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_HID;

  if (payload)
  {
    struct SingleHidDescriptor * const descriptor = payload;

    descriptor->base.hid = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->base.numDescriptors = 1;
    descriptor->entries[0].type = DESCRIPTOR_TYPE_HID_REPORT;
    descriptor->entries[0].length =
        toLittleEndian16(driver->reportDescriptorSize);
  }
}
/*----------------------------------------------------------------------------*/
static void endpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpointAddress;
    descriptor->attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_INTERRUPT);
    descriptor->maxPacketSize = toLittleEndian16(driver->reportDescriptorSize);
    descriptor->interval = 0x20; //TODO
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, const void *payload,
    uint16_t payloadLength, void *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  if (packet->index != driver->interfaceIndex)
    return E_VALUE;

  switch (packet->request)
  {
    case HID_REQUEST_GET_REPORT:
      usbTrace("hid at %u: get report type 0x%02X, id 0x%02X",
          driver->interfaceIndex, (uint8_t)(packet->value >> 8),
          (uint8_t)packet->value);

      return hidGetReport(driver->owner, packet->value >> 8,
          packet->value, response, responseLength,
          maxResponseLength);

    case HID_REQUEST_SET_REPORT:
      usbTrace("hid at %u: set report type 0x%02X, id 0x%02X, length %u",
          driver->interfaceIndex, (uint8_t)(packet->value >> 8),
          (uint8_t)packet->value, payloadLength);

      if (payloadLength)
      {
        return hidSetReport(driver->owner, packet->value >> 8, packet->value,
            payload, payloadLength);
      }
      else
        return E_VALUE;

    case HID_REQUEST_GET_IDLE:
      usbTrace("hid at %u: get idle time %u",
          driver->interfaceIndex, packet->value);

      ((uint8_t *)response)[0] = driver->idleTime;
      *responseLength = 1;
      return E_OK;

    case HID_REQUEST_SET_IDLE:
      usbTrace("hid at %u: set idle time to %u",
          driver->interfaceIndex, packet->value);

      driver->idleTime = packet->value >> 8;
      return E_OK;

    default:
      usbTrace("hid at %u: unknown request 0x%02X",
          driver->interfaceIndex, packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleDeviceRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, void *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum Result res = E_INVALID;

  if (packet->request == REQUEST_GET_DESCRIPTOR
      && DESCRIPTOR_TYPE(packet->value) == DESCRIPTOR_TYPE_HID_REPORT)
  {
    usbTrace("hid: get report descriptor, length %u",
        driver->reportDescriptorSize);

    if (driver->reportDescriptorSize <= maxResponseLength)
    {
      memcpy(response, driver->reportDescriptor,
          driver->reportDescriptorSize);
      *responseLength = driver->reportDescriptorSize;
      res = E_OK;
    }
    else
      res = E_VALUE;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct HidBaseConfig * const config = configBase;
  struct HidBase * const driver = object;

  if (!config->owner || !config->device)
    return E_VALUE;

  driver->owner = config->owner;
  driver->device = config->device;

  driver->reportDescriptor = config->descriptor;
  driver->reportDescriptorSize = config->descriptorSize;
  driver->idleTime = 0;

  driver->endpointAddress = config->endpoints.interrupt;
  driver->interfaceIndex = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct HidBase * const driver = object;

  usbDevUnbind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static enum Result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const void *payload,
    uint16_t payloadLength, void *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct HidBase * const driver = object;
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);
  enum Result res = E_INVALID;

  switch (type)
  {
    case REQUEST_TYPE_STANDARD:
      if (recipient == REQUEST_RECIPIENT_DEVICE)
      {
        res = handleDeviceRequest(driver, packet, response, responseLength,
            maxResponseLength);
      }
      break;

    case REQUEST_TYPE_CLASS:
      res = handleClassRequest(object, packet, payload, payloadLength,
          response, responseLength, maxResponseLength);
      break;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct HidBase * const driver = object;

  hidEvent(driver->owner, event);
}
