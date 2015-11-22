/*
 * cdc_acm_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/cdc_acm_base.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static void buildDescriptors(struct CdcAcmBase *,
    const struct CdcAcmBaseConfig *);
static void freeDescriptors(struct CdcAcmBase *);
static enum result handleRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static const struct UsbDescriptor **driverGetDescriptors(void *);
static void driverUpdateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CdcAcmBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .getDescriptors = driverGetDescriptors,
    .updateStatus = driverUpdateStatus
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcmBase = &driverTable;
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceDescriptor deviceDescriptor = {
    .length             = sizeof(struct UsbDeviceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_DEVICE,
    .usb                = TO_LITTLE_ENDIAN_16(0x0200),
    .deviceClass        = USB_CLASS_CDC,
    .deviceSubClass     = 0x00, /* Reserved value */
    .deviceProtocol     = 0x00, /* No class specific protocol */
    .maxPacketSize      = CDC_CONTROL_EP_SIZE,
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
        + sizeof(struct UsbInterfaceDescriptor) * 2
        + sizeof(struct UsbEndpointDescriptor) * 3
        + sizeof(struct CdcHeaderDescriptor)
        + sizeof(struct CdcCallManagementDescriptor)
        + sizeof(struct CdcAcmDescriptor)
        + sizeof(struct CdcUnionDescriptor)),
    .numInterfaces      = 2,
    .configurationValue = 1,
    .configuration      = 0, /* No configuration name */
    .attributes         = CONFIGURATION_DESCRIPTOR_DEFAULT
        | CONFIGURATION_DESCRIPTOR_SELF_POWERED,
    .maxPower           = ((CONFIG_USB_DEVICE_CURRENT + 1) >> 1)
};

static const struct UsbDescriptor * const controlDescriptors[] = {
    (const struct UsbDescriptor *)&(const struct UsbInterfaceDescriptor){
        .length             = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber    = 0,
        .alternateSettings  = 0,
        .numEndpoints       = 1,
        .interfaceClass     = USB_CLASS_CDC,
        .interfaceSubClass  = 0x02, /* Abstract Control Model */
        .interfaceProtocol  = 0x01, /* Common AT commands */
        .interface          = 0 /* No interface name */
    },
    (const struct UsbDescriptor *)&(const struct CdcHeaderDescriptor){
        .length             = sizeof(struct CdcHeaderDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType  = CDC_SUBTYPE_HEADER,
        .cdc                = TO_LITTLE_ENDIAN_16(0x0110)
    },
    (const struct UsbDescriptor *)&(const struct CdcCallManagementDescriptor){
        .length             = sizeof(struct CdcCallManagementDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType  = CDC_SUBTYPE_CALL_MANAGEMENT,
        .capabilities       = 0x01,
        .dataInterface      = 0x01
    },
    (const struct UsbDescriptor *)&(const struct CdcAcmDescriptor){
        .length             = sizeof(struct CdcAcmDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType  = CDC_SUBTYPE_ACM,
        .capabilities       = 0x02
    },
    (const struct UsbDescriptor *)&(const struct CdcUnionDescriptor){
        .length             = sizeof(struct CdcUnionDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType  = CDC_SUBTYPE_UNION,
        .masterInterface0   = 0x00,
        .slaveInterface0    = 0x01
    }
};

static const struct UsbDescriptor * const dataDescriptors[] = {
    (const struct UsbDescriptor *)&(const struct UsbInterfaceDescriptor){
        .length             = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
        .interfaceNumber    = 1,
        .alternateSettings  = 0,
        .numEndpoints       = 2,
        .interfaceClass     = USB_CLASS_CDC_DATA,
        .interfaceSubClass  = 0x00, /* None */
        .interfaceProtocol  = 0x00, /* None */
        .interface          = 0 /* No interface name */
    }
};

static const struct UsbDescriptor * const stringDescriptors[] = {
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
        (USB_STRING_PREFIX "Sample")
};
/*----------------------------------------------------------------------------*/
static void buildDescriptors(struct CdcAcmBase *driver,
    const struct CdcAcmBaseConfig *config)
{
  /*
   * 16 pointers to descriptors: device descriptor, configuration descriptor,
   * 9 interface descriptors, main string descriptor, 3 string descriptors
   * and one end marker.
   */
  driver->descriptorArray = malloc(16 * sizeof(struct UsbDescriptor *));
  driver->endpointDescriptors =
      malloc(3 * sizeof(struct UsbEndpointDescriptor));

  uint8_t index = 0;

  driver->descriptorArray[index++] =
      (const struct UsbDescriptor *)&deviceDescriptor;
  driver->descriptorArray[index++] =
      (const struct UsbDescriptor *)&configDescriptor;

  /* Copy control interface descriptors */
  for (uint8_t i = 0; i < ARRAY_SIZE(controlDescriptors); ++i)
    driver->descriptorArray[index++] = controlDescriptors[i];

  /* Notification endpoint */
  driver->endpointDescriptors[0].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[0].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[0].endpointAddress = config->endpoint.interrupt;
  driver->endpointDescriptors[0].attributes = 0x03;
  driver->endpointDescriptors[0].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_NOTIFICATION_EP_SIZE);
  driver->endpointDescriptors[0].interval = 8;

  driver->descriptorArray[index++] =
      (struct UsbDescriptor *)&driver->endpointDescriptors[0];

  /* Copy data interface descriptors */
  for (uint8_t i = 0; i < ARRAY_SIZE(dataDescriptors); ++i)
    driver->descriptorArray[index++] = dataDescriptors[i];

  /* Bulk transmit endpoint */
  driver->endpointDescriptors[1].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[1].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[1].endpointAddress = config->endpoint.tx;
  driver->endpointDescriptors[1].attributes = 0x02;
  driver->endpointDescriptors[1].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  driver->endpointDescriptors[1].interval = 0;

  /* Bulk receive endpoint */
  driver->endpointDescriptors[2].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[2].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[2].endpointAddress = config->endpoint.rx;
  driver->endpointDescriptors[2].attributes = 0x02;
  driver->endpointDescriptors[2].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  driver->endpointDescriptors[2].interval = 0;

  driver->descriptorArray[index++] =
      (struct UsbDescriptor *)&driver->endpointDescriptors[1];
  driver->descriptorArray[index++] =
      (struct UsbDescriptor *)&driver->endpointDescriptors[2];

  /* Copy string descriptors */
  for (uint8_t i = 0; i < ARRAY_SIZE(stringDescriptors); ++i)
    driver->descriptorArray[index + i] = stringDescriptors[i];

  if (config->serial)
  {
    const unsigned int serialLength = strlen(config->serial) + 1;

    assert(serialLength < 127);

    /* 2 bytes for default header and 1 byte for terminating character */
    driver->stringDescriptor = malloc(2 + serialLength);
    driver->stringDescriptor->length = 0;
    driver->stringDescriptor->descriptorType = DESCRIPTOR_TYPE_STRING;
    memcpy(driver->stringDescriptor->data, config->serial, serialLength);

    driver->descriptorArray[index + deviceDescriptor.serialNumber] =
        driver->stringDescriptor;
  }
  else
  {
    driver->stringDescriptor = 0;
  }

  index += ARRAY_SIZE(stringDescriptors);

  /* Add end of the array mark */
  driver->descriptorArray[index] = 0;
}
/*----------------------------------------------------------------------------*/
static void freeDescriptors(struct CdcAcmBase *driver)
{
  if (driver->stringDescriptor)
    free(driver->stringDescriptor);
  free(driver->endpointDescriptors);
  free(driver->descriptorArray);
}
/*----------------------------------------------------------------------------*/
static enum result handleRequest(struct CdcAcmBase *driver,
    const struct UsbSetupPacket *packet, const uint8_t *input,
    uint16_t inputLength, uint8_t *output, uint16_t *outputLength,
    uint16_t maxOutputLength)
{
  bool event = false;

  switch (packet->request)
  {
    case CDC_SET_LINE_CODING:
      if (inputLength != sizeof(driver->line.coding))
        return E_VALUE;

      memcpy(&driver->line.coding, input, sizeof(driver->line.coding));
      event = true;
      *outputLength = 0;

      usbTrace("cdc_acm: rate %u, format %u, parity %u, width %u",
          driver->line.coding.dteRate, driver->line.coding.charFormat,
          driver->line.coding.parityType, driver->line.coding.dataBits);
      break;

    case CDC_GET_LINE_CODING:
      if (maxOutputLength < sizeof(driver->line.coding))
        return E_VALUE;

      memcpy(output, &driver->line.coding, sizeof(driver->line.coding));
      *outputLength = sizeof(driver->line.coding);

      usbTrace("cdc_acm: line coding requested");
      break;

    case CDC_SET_CONTROL_LINE_STATE:
    {
      const bool dtrState = (packet->value & 0x01) != 0;
      const bool rtsState = (packet->value & 0x02) != 0;

      driver->line.dtr = dtrState;
      driver->line.rts = rtsState;
      event = true;
      *outputLength = 0;

      usbTrace("cdc_acm: set control lines to %02X", packet->value);
      break;
    }

    default:
      usbTrace("cdc_acm: unknown request %02X", packet->request);
      return E_INVALID;
  }

  if (event)
  {
    driver->line.updated = true;
    if (driver->callback)
      driver->callback(driver->callbackArgument);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CdcAcmBaseConfig * const config = configBase;
  struct CdcAcmBase * const driver = object;
  enum result res;

  driver->callback = config->callback;
  driver->callbackArgument = config->argument;
  driver->suspended = false;

  driver->device = config->device;
  driver->line.coding = (struct CdcLineCoding){115200, 0, 0, 8};
  driver->line.dtr = true;
  driver->line.rts = true;
  driver->line.updated = false;

  buildDescriptors(driver, config);

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

  usbDevSetConnected(driver->device, true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CdcAcmBase * const driver = object;

  usbDevSetConnected(driver->device, false);
  usbDevBind(driver->device, 0); /* Unbind driver */
  freeDescriptors(driver);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct CdcAcmBase * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_CLASS)
    return E_INVALID;

  const enum result res = handleRequest(driver, packet, payload, payloadLength,
      response, responseLength, maxResponseLength);

  return res;
}
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor **driverGetDescriptors(void *object)
{
  const struct CdcAcmBase * const driver = object;

  return driver->descriptorArray;
}
/*----------------------------------------------------------------------------*/
static void driverUpdateStatus(void *object, uint8_t status)
{
  struct CdcAcmBase * const driver = object;

  driver->suspended = (status & DEVICE_STATUS_SUSPEND) != 0;
  if (driver->callback)
    driver->callback(driver->callbackArgument);
}
