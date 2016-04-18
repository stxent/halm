/*
 * hid_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/composite_device.h>
#include <usb/hid.h>
#include <usb/hid_base.h>
#include <usb/hid_defs.h>
#include <usb/usb_defs.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
struct SingleHidDescriptor
{
  struct HidDescriptorBase base;
  struct HidDescriptorEntry entries[1];
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct PrivateData
{
  struct SingleHidDescriptor hidDescriptor;
  struct UsbEndpointDescriptor endpointDescriptor;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  struct UsbInterfaceDescriptor interfaceDescriptor;
#endif
};
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct HidBase *,
    const struct HidBaseConfig *);
static enum result descriptorEraseWrapper(void *, const void *);
static enum result iterateOverDescriptors(struct HidBase *,
    enum result (*)(void *, const void *));
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct HidBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const HidBase = &driverTable;
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_USB_DEVICE_COMPOSITE
static const struct UsbDeviceDescriptor deviceDescriptor = {
    .length             = sizeof(struct UsbDeviceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_DEVICE,
    .usb                = TO_LITTLE_ENDIAN_16(0x0200),
    .deviceClass        = USB_CLASS_PER_INTERFACE,
    .deviceSubClass     = 0x00, /* Reserved value */
    .deviceProtocol     = 0x00, /* No class specific protocol */
    .maxPacketSize      = HID_CONTROL_EP_SIZE,
    .idVendor           = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_VENDOR_ID),
    .idProduct          = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_PRODUCT_ID),
    .device             = TO_LITTLE_ENDIAN_16(0x0100),
    .manufacturer       = 0,
    .product            = 0,
    .serialNumber       = 0,
    .numConfigurations  = 1
};

static const struct UsbConfigurationDescriptor configDescriptor = {
    .length             = sizeof(struct UsbConfigurationDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_CONFIGURATION,
    .totalLength        = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct SingleHidDescriptor)
        + sizeof(struct UsbEndpointDescriptor)),
    .numInterfaces      = 1,
    .configurationValue = 1,
    .configuration      = 0, /* No configuration name */
    .attributes         = CONFIGURATION_DESCRIPTOR_DEFAULT
        | CONFIGURATION_DESCRIPTOR_SELF_POWERED,
    .maxPower           = ((CONFIG_USB_DEVICE_CURRENT + 1) >> 1)
};

static const struct UsbInterfaceDescriptor interfaceDescriptor = {
    .length             = sizeof(struct UsbInterfaceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
    .interfaceNumber    = 0,
    .alternateSettings  = 0,
    .numEndpoints       = 1,
    .interfaceClass     = USB_CLASS_HID,
    .interfaceSubClass  = HID_SUBCLASS_NONE,
    .interfaceProtocol  = HID_PROTOCOL_NONE,
    .interface          = 0 /* No interface name */
};
#endif
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct HidBase *driver,
    const struct HidBaseConfig *config)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));

  if (!privateData)
    return E_MEMORY;
  driver->privateData = privateData;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const uint8_t interfaceIndex = usbCompositeDevIndex(driver->device);

  driver->interfaceIndex = interfaceIndex;

  privateData->interfaceDescriptor.length =
      sizeof(struct UsbInterfaceDescriptor);
  privateData->interfaceDescriptor.descriptorType = DESCRIPTOR_TYPE_INTERFACE;
  privateData->interfaceDescriptor.interfaceNumber = interfaceIndex;
  privateData->interfaceDescriptor.alternateSettings = 0;
  privateData->interfaceDescriptor.numEndpoints = 1;
  privateData->interfaceDescriptor.interfaceClass = USB_CLASS_HID;
  privateData->interfaceDescriptor.interfaceSubClass = HID_SUBCLASS_NONE;
  privateData->interfaceDescriptor.interfaceProtocol = HID_PROTOCOL_NONE;
  privateData->interfaceDescriptor.interface = 0;
#endif

  /* HID descriptor */
  privateData->hidDescriptor.base.length = sizeof(struct SingleHidDescriptor);
  privateData->hidDescriptor.base.descriptorType = DESCRIPTOR_TYPE_HID;
  privateData->hidDescriptor.base.hid = TO_LITTLE_ENDIAN_16(0x0100);
  privateData->hidDescriptor.base.countryCode = 0;
  privateData->hidDescriptor.base.numDescriptors = 1;
  privateData->hidDescriptor.entries[0].type = DESCRIPTOR_TYPE_HID_REPORT;
  privateData->hidDescriptor.entries[0].length =
      toLittleEndian16(config->descriptorSize);

  /* Interrupt endpoint */
  privateData->endpointDescriptor.length = sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptor.descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptor.endpointAddress = config->endpoint.interrupt;
  privateData->endpointDescriptor.attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_INTERRUPT);
  privateData->endpointDescriptor.maxPacketSize =
      toLittleEndian16(config->reportSize);
  privateData->endpointDescriptor.interval = 0x20; //TODO

  return iterateOverDescriptors(driver, usbDevAppendDescriptor);
}
/*----------------------------------------------------------------------------*/
static enum result descriptorEraseWrapper(void *device, const void *descriptor)
{
  usbDevEraseDescriptor(device, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result iterateOverDescriptors(struct HidBase *driver,
    enum result (*action)(void *, const void *))
{
  struct PrivateData * const privateData = driver->privateData;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const void * const descriptors[] = {
      &privateData->interfaceDescriptor,
      &privateData->hidDescriptor,
      &privateData->endpointDescriptor
  };
#else
  const void * const descriptors[] = {
      &deviceDescriptor,
      &configDescriptor,
      &interfaceDescriptor,
      &privateData->hidDescriptor,
      &privateData->endpointDescriptor
  };
#endif

  for (unsigned int i = 0; i < ARRAY_SIZE(descriptors); ++i)
  {
    const enum result res = action(driver->device, descriptors[i]);

    if (res != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct HidBaseConfig * const config = configBase;
  struct HidBase * const driver = object;
  enum result res;

  if (!config->owner || !config->device)
    return E_VALUE;

  driver->owner = config->owner;
  driver->device = config->device;
  driver->reportDescriptor = config->descriptor;
  driver->reportDescriptorSize = config->descriptorSize;
  driver->idleTime = 0;
  driver->interfaceIndex = 0;

  if ((res = buildDescriptors(driver, config)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct HidBase * const driver = object;

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, false);
#endif

  usbDevUnbind(driver->device, driver);
  iterateOverDescriptors(driver, descriptorEraseWrapper);
  free(driver->privateData);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct HidBase * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (packet->index != driver->interfaceIndex)
    return E_INVALID;

  if (type == REQUEST_TYPE_STANDARD)
  {
    if (packet->request == REQUEST_GET_DESCRIPTOR)
    {
      if (DESCRIPTOR_TYPE(packet->value) != DESCRIPTOR_TYPE_HID_REPORT)
        return E_INVALID;

      usbTrace("hid: get report descriptor, length %u",
          driver->reportDescriptorSize);

      if (driver->reportDescriptorSize > maxResponseLength)
        return E_VALUE;

      memcpy(response, driver->reportDescriptor, driver->reportDescriptorSize);
      *responseLength = driver->reportDescriptorSize;
      return E_OK;
    }
    else
    {
      return E_INVALID;
    }
  }
  else if (type == REQUEST_TYPE_CLASS)
  {
    switch (packet->request)
    {
      case HID_REQUEST_GET_REPORT:
        usbTrace("hid: get report type 0x%02X, id 0x%02X",
            (uint8_t)(packet->value >> 8), (uint8_t)packet->value);

        if (!maxResponseLength)
          return E_VALUE;

        return hidGetReport(driver->owner, (uint8_t)(packet->value >> 8),
            (uint8_t)packet->value, response, responseLength,
            maxResponseLength);

      case HID_REQUEST_SET_REPORT:
        usbTrace("hid: set report type 0x%02X, id 0x%02X, length %u",
            (uint8_t)(packet->value >> 8), (uint8_t)packet->value,
            payloadLength);

        if (!payloadLength)
          return E_VALUE;

        return hidSetReport(driver->owner, (uint8_t)(packet->value >> 8),
            (uint8_t)packet->value, payload, payloadLength);

      case HID_REQUEST_GET_IDLE:
        usbTrace("hid: get idle time %u", packet->value);

        if (maxResponseLength < 1)
          return E_VALUE;
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
  else
  {
    return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct HidBase * const driver = object;

  hidEvent(driver->owner, event);
}
