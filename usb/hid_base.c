/*
 * hid_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/hid.h>
#include <halm/usb/hid_base.h>
#include <halm/usb/hid_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] SingleHidDescriptor
{
  struct HidDescriptorBase base;
  struct HidDescriptorEntry entries[1];
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void interfaceDescriptor(const void *, struct UsbDescriptor *, void *);
static void hidDescriptor(const void *, struct UsbDescriptor *, void *);
static void endpointDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct HidBase *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum Result handleDeviceRequest(struct HidBase *,
    const struct UsbSetupPacket *, void *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const HidBase = &(const struct UsbDriverClass){
    .size = sizeof(struct HidBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .control = driverControl,
    .describe = driverDescribe,
    .notify = driverNotify
};
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor deviceDescriptorTable[] = {
    deviceDescriptor,
    configDescriptor,
    interfaceDescriptor,
    hidDescriptor,
    endpointDescriptor,
    NULL
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbDeviceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_DEVICE;

  if (payload != NULL)
  {
    static const struct UsbDeviceDescriptor descriptor = {
        .length = sizeof(struct UsbDeviceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_DEVICE,
        .usb = TO_LITTLE_ENDIAN_16(0x0200),
        .deviceClass = USB_CLASS_PER_INTERFACE,
        .deviceSubClass = 0,
        .deviceProtocol = 0,
        .maxPacketSize = TO_LITTLE_ENDIAN_16(HID_CONTROL_EP_SIZE),
        .idVendor = 0,
        .idProduct = 0,
        .device = TO_LITTLE_ENDIAN_16(0x0100),
        .manufacturer = 0,
        .product = 0,
        .serialNumber = 0,
        .numConfigurations = 1
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *, struct UsbDescriptor *header,
    void *payload)
{
  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (payload != NULL)
  {
    static const struct UsbConfigurationDescriptor descriptor = {
        .length = sizeof(struct UsbConfigurationDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_CONFIGURATION,
        .totalLength = TO_LITTLE_ENDIAN_16(
            sizeof(struct UsbConfigurationDescriptor)
            + sizeof(struct UsbInterfaceDescriptor)
            + sizeof(struct SingleHidDescriptor)
            + sizeof(struct UsbEndpointDescriptor)),
        .numInterfaces = 1,
        .configurationValue = 1,
        .configuration = 0,
        .attributes = 0,
        .maxPower = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload != NULL)
  {
    const struct UsbInterfaceDescriptor descriptor = {
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber = driver->interfaceIndex,
        .alternateSettings = 0,
        .numEndpoints = 1,
        .interfaceClass = USB_CLASS_HID,
        .interfaceSubClass = HID_SUBCLASS_NONE,
        .interfaceProtocol = HID_PROTOCOL_NONE,
        .interface = 0
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void hidDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct SingleHidDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_HID;

  if (payload != NULL)
  {
    struct SingleHidDescriptor descriptor = {
        .base = {
            .length = sizeof(struct SingleHidDescriptor),
            .descriptorType = DESCRIPTOR_TYPE_HID,
            .hid = TO_LITTLE_ENDIAN_16(0x0100),
            .countryCode = 0,
            .numDescriptors = 1
        },
        .entries = {
            {
                .type = DESCRIPTOR_TYPE_HID_REPORT,
                .length = toLittleEndian16(driver->reportDescriptorSize)
            }
        }
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static void endpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct HidBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload != NULL)
  {
    const struct UsbEndpointDescriptor descriptor = {
        .length = sizeof(struct UsbEndpointDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_ENDPOINT,
        .endpointAddress = driver->endpointAddress,
        .attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_INTERRUPT),
        .maxPacketSize = toLittleEndian16(driver->reportDescriptorSize),
        .interval = 0x20 /* TODO */
    };

    memcpy(payload, &descriptor, sizeof(descriptor));
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  if (packet->index != driver->interfaceIndex)
    return E_INVALID;

  enum Result res;

  switch (packet->request)
  {
    case HID_REQUEST_GET_REPORT:
    {
      usbTrace("hid at %u: get report type 0x%02X, id 0x%02X",
          driver->interfaceIndex, (uint8_t)(packet->value >> 8),
          (uint8_t)packet->value);

      res = hidGetReport(driver->owner, packet->value >> 8,
          packet->value, response, responseLength,
          maxResponseLength);
      break;
    }

    case HID_REQUEST_SET_REPORT:
    {
      usbTrace("hid at %u: set report type 0x%02X, id 0x%02X, length %u",
          driver->interfaceIndex, (uint8_t)(packet->value >> 8),
          (uint8_t)packet->value, packet->length);

      if (packet->length)
      {
        res = hidSetReport(driver->owner, packet->value >> 8, packet->value,
            response, packet->length);
      }
      else
        res = E_VALUE;
      break;
    }

    case HID_REQUEST_GET_IDLE:
    {
      usbTrace("hid at %u: get idle time %u",
          driver->interfaceIndex, packet->value);

      response[0] = driver->idleTime;
      *responseLength = 1;
      res = E_OK;
      break;
    }

    case HID_REQUEST_SET_IDLE:
    {
      usbTrace("hid at %u: set idle time to %u",
          driver->interfaceIndex, packet->value);

      driver->idleTime = packet->value >> 8;
      res = E_OK;
      break;
    }

    default:
    {
      usbTrace("hid at %u: unknown request 0x%02X",
          driver->interfaceIndex, packet->request);
      res = E_INVALID;
      break;
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result handleDeviceRequest(struct HidBase *driver,
    const struct UsbSetupPacket *packet, void *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum Result res;

  if (packet->request == REQUEST_GET_DESCRIPTOR
      && DESCRIPTOR_TYPE(packet->value) == DESCRIPTOR_TYPE_HID_REPORT)
  {
    usbTrace("hid: get report descriptor, length %u",
        driver->reportDescriptorSize);

    if (driver->reportDescriptorSize <= maxResponseLength)
    {
      memcpy(response, driver->reportDescriptor, driver->reportDescriptorSize);
      *responseLength = driver->reportDescriptorSize;
      res = E_OK;
    }
    else
      res = E_VALUE;
  }
  else
    res = E_INVALID;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct HidBaseConfig * const config = configBase;
  assert(config->owner != NULL);
  assert(config->device != NULL);

  struct HidBase * const driver = object;

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
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength, uint16_t maxResponseLength)
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
        res = handleDeviceRequest(driver, packet, buffer, responseLength,
            maxResponseLength);
      }
      break;

    case REQUEST_TYPE_CLASS:
      res = handleClassRequest(object, packet, buffer, responseLength,
          maxResponseLength);
      break;

    default:
      break;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *)
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct HidBase * const driver = object;
  hidEvent(driver->owner, event);
}
