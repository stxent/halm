/*
 * hid_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/hid.h>
#include <usb/hid_base.h>
#include <usb/hid_defs.h>
#include <usb/usb_requests.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define HID_CONTROL_EP_SIZE 64
#define HID_DESCRIPTOR_SIZE \
    (HID_DESCRIPTOR_BASE_SIZE + HID_DESCRIPTOR_ENTRY_SIZE)
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct HidBase *,
    const struct HidBaseConfig *);
static inline void freeDescriptors(struct HidBase *);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverUpdateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct HidBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .updateStatus = driverUpdateStatus
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const HidBase = &driverTable;
/*----------------------------------------------------------------------------*/
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
        + sizeof(struct UsbEndpointDescriptor)
        + HID_DESCRIPTOR_SIZE),
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
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct HidBase *driver,
    const struct HidBaseConfig *config)
{
  /*
   * 5 pointers to descriptors:
   *   1 device descriptor,
   *   1 configuration descriptor,
   *   1 interface descriptor,
   *   1 endpoint descriptor,
   *   1 class specific descriptor.
   */

  driver->hidDescriptor = malloc(HID_DESCRIPTOR_SIZE);
  if (!driver->hidDescriptor)
    return E_MEMORY;

  driver->endpointDescriptor = malloc(sizeof(struct UsbEndpointDescriptor));
  if (!driver->endpointDescriptor)
    return E_MEMORY;

  enum result res;

  res = usbDevAppendDescriptor(driver->device, &deviceDescriptor);
  if (res != E_OK)
    return res;
  res = usbDevAppendDescriptor(driver->device, &configDescriptor);
  if (res != E_OK)
    return res;
  res = usbDevAppendDescriptor(driver->device, &interfaceDescriptor);
  if (res != E_OK)
    return res;

  /* HID descriptor */
  driver->hidDescriptor->length = HID_DESCRIPTOR_SIZE;
  driver->hidDescriptor->descriptorType = DESCRIPTOR_TYPE_HID;
  driver->hidDescriptor->hid = TO_LITTLE_ENDIAN_16(0x0100);
  driver->hidDescriptor->countryCode = 0;
  driver->hidDescriptor->numDescriptors = 1;
  driver->hidDescriptor->classDescriptors[0].type = DESCRIPTOR_TYPE_HID_REPORT;
  driver->hidDescriptor->classDescriptors[0].length =
      toLittleEndian16(config->reportSize);

  res = usbDevAppendDescriptor(driver->device, driver->hidDescriptor);
  if (res != E_OK)
    return res;

  /* Interrupt endpoint */
  driver->endpointDescriptor->length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptor->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptor->endpointAddress = config->endpoint.interrupt;
  driver->endpointDescriptor->attributes = ENDPOINT_DESCRIPTOR_INTERRUPT;
  driver->endpointDescriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(4); //TODO
  driver->endpointDescriptor->interval = 0x20; //TODO

  res = usbDevAppendDescriptor(driver->device, driver->endpointDescriptor);
  if (res != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static inline void freeDescriptors(struct HidBase *driver)
{
  usbDevEraseDescriptor(driver->device, driver->endpointDescriptor);
  usbDevEraseDescriptor(driver->device, driver->hidDescriptor);
  usbDevEraseDescriptor(driver->device, &interfaceDescriptor);
  usbDevEraseDescriptor(driver->device, &configDescriptor);
  usbDevEraseDescriptor(driver->device, &deviceDescriptor);

  free(driver->endpointDescriptor);
  free(driver->hidDescriptor);
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
  driver->reportDescriptor = config->report;
  driver->reportDescriptorSize = config->reportSize;
  driver->idleTime = 0;

  if ((res = buildDescriptors(driver, config)) != E_OK)
    return res;

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

  usbDevSetConnected(driver->device, true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct HidBase * const driver = object;

  usbDevSetConnected(driver->device, false);
  usbDevUnbind(driver->device, driver);
  freeDescriptors(driver);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct HidBase * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

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
        usbTrace("hid: get report type %02X, id %02X",
            (uint8_t)(packet->value >> 8), (uint8_t)packet->value);

        if (!maxResponseLength)
          return E_VALUE;

        return hidGetReport(driver->owner, (uint8_t)(packet->value >> 8),
            (uint8_t)packet->value, response, responseLength,
            maxResponseLength);

      case HID_REQUEST_SET_REPORT:
        usbTrace("hid: set report type %02X, id %02X, length %u",
            (uint8_t)(packet->value >> 8), (uint8_t)packet->value,
            payloadLength);

        if (!payloadLength)
          return E_VALUE;

        return hidSetReport(driver->owner, (uint8_t)(packet->value >> 8),
            (uint8_t)packet->value, payload, payloadLength);

      case HID_REQUEST_GET_IDLE:
        usbTrace("hid: get idle time %04X", packet->value);

        if (maxResponseLength < 1)
          return E_VALUE;
        response[0] = driver->idleTime;
        *responseLength = 1;
        return E_OK;

      case HID_REQUEST_SET_IDLE:
        usbTrace("hid: set idle time to %04X", packet->value);

        driver->idleTime = (uint8_t)(packet->value >> 8); //TODO Check
        return E_OK;

      default:
        usbTrace("hid: unknown request %02X", packet->request);
        return E_INVALID;
    }
  }
  else
  {
    return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static void driverUpdateStatus(void *object, uint8_t status)
{
  struct HidBase * const driver = object;

  hidUpdateStatus(driver->owner, status);
}
