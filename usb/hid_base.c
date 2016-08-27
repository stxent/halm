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
#include <halm/usb/usb_requests.h>
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
static enum result handleClassRequest(struct HidBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t,
    uint8_t *, uint16_t *, uint16_t);
static enum result handleDeviceRequest(struct HidBase *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
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
static void deviceDescriptor(const void *object, struct UsbDescriptor *header,
    void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload)
  {
    struct UsbDeviceDescriptor * const descriptor = payload;

    usbFillDeviceDescriptor(driver->device, descriptor);
    descriptor->usb = TO_LITTLE_ENDIAN_16(0x0200);
    descriptor->deviceClass = USB_CLASS_PER_INTERFACE;
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(HID_CONTROL_EP_SIZE);
    descriptor->device = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->numConfigurations = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *object, struct UsbDescriptor *header,
    void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload)
  {
    struct UsbConfigurationDescriptor * const descriptor = payload;

    usbFillConfigurationDescriptor(driver->device, descriptor);
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
static enum result handleClassRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  if (packet->index != driver->interfaceIndex)
    return E_VALUE;

  switch (packet->request)
  {
    case HID_REQUEST_GET_REPORT:
      usbTrace("hid: get report type 0x%02X, id 0x%02X",
          (uint8_t)(packet->value >> 8), (uint8_t)packet->value);

      return hidGetReport(driver->owner, (uint8_t)(packet->value >> 8),
          (uint8_t)packet->value, response, responseLength,
          maxResponseLength);

    case HID_REQUEST_SET_REPORT:
      usbTrace("hid: set report type 0x%02X, id 0x%02X, length %u",
          (uint8_t)(packet->value >> 8), (uint8_t)packet->value,
          payloadLength);

      if (payloadLength)
      {
        return hidSetReport(driver->owner, (uint8_t)(packet->value >> 8),
            (uint8_t)packet->value, payload, payloadLength);
      }
      else
        return E_VALUE;

    case HID_REQUEST_GET_IDLE:
      usbTrace("hid: get idle time %u", packet->value);

      response[0] = driver->idleTime;
      *responseLength = 1;
      return E_OK;

    case HID_REQUEST_SET_IDLE:
      usbTrace("hid: set idle time to %u", packet->value);

      driver->idleTime = (uint8_t)(packet->value >> 8);
      return E_OK;

    default:
      usbTrace("hid: unknown request 0x%02X", packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result handleDeviceRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_DESCRIPTOR:
    {
      if (DESCRIPTOR_TYPE(packet->value) == DESCRIPTOR_TYPE_HID_REPORT)
      {
        usbTrace("hid: get report descriptor, length %u",
            driver->reportDescriptorSize);

        if (driver->reportDescriptorSize > maxResponseLength)
          return E_VALUE;

        memcpy(response, driver->reportDescriptor,
            driver->reportDescriptorSize);
        *responseLength = driver->reportDescriptorSize;
        return E_OK;
      }
      else
      {
        usbTrace("hid: get descriptor %u:%u, length %u",
            DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
            packet->length);

        return usbExtractDescriptorData(driver, packet->value, packet->index,
            response, responseLength, maxResponseLength);
      }
    }

    default:
      return usbHandleDeviceRequest(driver, driver->device, packet,
          response, responseLength);
  }
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
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
  driver->composite = config->composite;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct HidBase * const driver = object;

  if (!driver->composite)
    usbDevSetConnected(driver->device, false);

  usbDevUnbind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct HidBase * const driver = object;
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type == REQUEST_TYPE_STANDARD)
  {
    switch (recipient)
    {
      case REQUEST_RECIPIENT_DEVICE:
        return handleDeviceRequest(driver, packet, response, responseLength,
            maxResponseLength);

      case REQUEST_RECIPIENT_INTERFACE:
        if (packet->index == driver->interfaceIndex)
          return usbHandleInterfaceRequest(packet, response, responseLength);
        else
          return E_INVALID;

      case REQUEST_RECIPIENT_ENDPOINT:
        return usbHandleEndpointRequest(driver->device, packet, response,
            responseLength);

      default:
        return E_INVALID;
    }
  }
  else if (type == REQUEST_TYPE_CLASS)
  {
    return handleClassRequest(object, packet, payload, payloadLength, response,
        responseLength, maxResponseLength);
  }
  else
    return E_INVALID;
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
