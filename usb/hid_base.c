/*
 * hid_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
#include <memory.h>
#include <usb/hid.h>
#include <usb/hid_base.h>
#include <usb/hid_defs.h>
#include <usb/usb_requests.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define HID_CONTROL_EP_SIZE 64
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static const struct UsbDescriptor **driverGetDescriptors(void *);
static void driverUpdateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct HidBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .getDescriptors = driverGetDescriptors,
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
    .manufacturer       = 1,
    .product            = 2,
    .serialNumber       = 3,
    .numConfigurations  = 1
};

static const struct UsbConfigurationDescriptor configDescriptor = {
    .length             = sizeof(struct UsbConfigurationDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_CONFIGURATION,
    .totalLength        = TO_LITTLE_ENDIAN_16(
        sizeof(struct UsbConfigurationDescriptor)
        + sizeof(struct UsbInterfaceDescriptor)
        + sizeof(struct UsbEndpointDescriptor)
        + sizeof(struct HidDescriptor)),
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
    .interfaceSubClass  = 0, //TODO
    .interfaceProtocol  = 0, //TODO
    .interface          = 0 /* No interface name */
};

const uint8_t HID_ReportDescriptor[] = {
  REPORT_USAGE_PAGE_VENDOR(0x00),
  REPORT_USAGE(0x01),
  REPORT_COLLECTION(HID_APPLICATION),
    REPORT_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
    REPORT_USAGE_MIN(1),
    REPORT_USAGE_MAX(3),
    REPORT_LOGICAL_MIN(0),
    REPORT_LOGICAL_MAX(1),
    REPORT_REPORT_COUNT(3),
    REPORT_REPORT_SIZE(1),
    REPORT_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    REPORT_REPORT_COUNT(1),
    REPORT_REPORT_SIZE(5),
    REPORT_INPUT(HID_CONSTANT),
    REPORT_USAGE_PAGE(HID_USAGE_PAGE_LED),
    REPORT_USAGE(HID_USAGE_LED_GENERIC_INDICATOR),
    REPORT_LOGICAL_MIN(0),
    REPORT_LOGICAL_MAX(1),
    REPORT_REPORT_COUNT(8),
    REPORT_REPORT_SIZE(1),
    REPORT_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
  REPORT_END_COLLECTION
};

static const struct HidDescriptor hidDescriptor = {
    .length                     = sizeof(struct HidDescriptor),
    .descriptorType             = DESCRIPTOR_TYPE_HID,
    .hid                        = TO_LITTLE_ENDIAN_16(0x0100),
    .countryCode                = 0,
    .numDescriptors             = 1,
    .xtype    = DESCRIPTOR_TYPE_HID_REPORT,
    .xlength  = sizeof(HID_ReportDescriptor)
//    .optionalDescriptor = {
//        .type    = DESCRIPTOR_TYPE_HID_REPORT,
//        .length  = sizeof(HID_ReportDescriptor)
//    }
};

static const struct UsbEndpointDescriptor intDescriptor = {
    .length             = sizeof(struct UsbEndpointDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_ENDPOINT,
    .endpointAddress    = 0x81,
    .attributes         = ENDPOINT_DESCRIPTOR_INTERRUPT,
    .maxPacketSize      = TO_LITTLE_ENDIAN_16(4),
    .interval           = 0x20
};

//static const struct UsbDescriptor * const stringDescriptors[] = {
//
//
//};
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor *descriptorArray[] = {
    (void *)&deviceDescriptor,
    (void *)&configDescriptor,
    (void *)&interfaceDescriptor,
    (void *)&hidDescriptor,
    (void *)&intDescriptor,
    (const struct UsbDescriptor *)&(const struct UsbStringDescriptor){
        .length         = sizeof(struct UsbStringDescriptor),
        .descriptorType = DESCRIPTOR_TYPE_STRING,
        .langid         = TO_LITTLE_ENDIAN_16(LANGID_ENGLISH_UK)
    },
    (const struct UsbDescriptor *)
        (USB_STRING_PREFIX EXPAND_TO_STRING(CONFIG_USB_DEVICE_VENDOR_NAME)),
    (const struct UsbDescriptor *)
        (USB_STRING_PREFIX EXPAND_TO_STRING(CONFIG_USB_DEVICE_PRODUCT_NAME)),
    (const struct UsbDescriptor *)
        (USB_STRING_PREFIX "Sample"),
    0
};
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
  driver->descriptorArray = descriptorArray;
  driver->reportDescriptor = config->report;
  driver->reportDescriptorSize = config->reportSize;

  driver->idleTime = 0;
  driver->protocol = 0;

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
  usbDevBind(driver->device, 0); /* Unbind driver */
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

//      case HID_REQUEST_GET_PROTOCOL:
//        if (maxResponseLength < 1)
//          return E_VALUE;
//        usbTrace("hid: get protocol %04X", packet->value);
//        response[0] = driver->protocol;
//        *responseLength = 1;
//        return E_OK;
//
//      case HID_REQUEST_SET_PROTOCOL:
//        usbTrace("hid: set protocol to %04X", packet->value);
//        driver->protocol = (uint8_t)(packet->value >> 8); //TODO Check
//        return E_OK;

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
static const struct UsbDescriptor **driverGetDescriptors(void *object)
{
  struct HidBase * const driver = object;

  return driver->descriptorArray;
}
/*----------------------------------------------------------------------------*/
static void driverUpdateStatus(void *object __attribute__((unused)), uint8_t status __attribute__((unused)))
{

}
