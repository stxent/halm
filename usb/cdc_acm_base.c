/*
 * cdc_acm_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/cdc_acm.h>
#include <usb/cdc_acm_base.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct CdcAcmBase *,
    const struct CdcAcmBaseConfig *);
static inline void freeDescriptors(struct CdcAcmBase *);
static enum result handleRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverUpdateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CdcAcmBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .updateStatus = driverUpdateStatus
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcmBase = &driverTable;
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_USB_COMPOSITE
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

static const struct UsbInterfaceDescriptor controlInterface = {
    .length             = sizeof(struct UsbInterfaceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
    .interfaceNumber    = 0,
    .alternateSettings  = 0,
    .numEndpoints       = 1,
    .interfaceClass     = USB_CLASS_CDC,
    .interfaceSubClass  = 0x02, /* Abstract Control Model */
    .interfaceProtocol  = 0x00, /* No protocol used */
    .interface          = 0 /* No interface name */
};

static const struct UsbInterfaceDescriptor dataInterface = {
    .length             = sizeof(struct UsbInterfaceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_INTERFACE,
    .interfaceNumber    = 1,
    .alternateSettings  = 0,
    .numEndpoints       = 2,
    .interfaceClass     = USB_CLASS_CDC_DATA,
    .interfaceSubClass  = 0x00, /* None */
    .interfaceProtocol  = 0x00, /* None */
    .interface          = 0 /* No interface name */
};
#endif
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor * const controlDescriptors[] = {
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
//    (const struct UsbDescriptor *)&(const struct CdcUnionDescriptor){
//        .length             = sizeof(struct CdcUnionDescriptor),
//        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
//        .descriptorSubType  = CDC_SUBTYPE_UNION,
//        .masterInterface0   = 0x00,
//        .slaveInterface0    = 0x01
//    }
};
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct CdcAcmBase *driver,
    const struct CdcAcmBaseConfig *config)
{
  /*
   * 11 pointers to descriptors:
   *   1 device descriptor,
   *   1 configuration descriptor,
   *   2 interface descriptors,
   *   3 endpoint descriptors,
   *   4 class specific descriptors.
   */

  enum result res;

#ifndef CONFIG_USB_COMPOSITE
  res = usbDevAppendDescriptor(driver->device, &deviceDescriptor);
  if (res != E_OK)
    return res;

  res = usbDevAppendDescriptor(driver->device, &configDescriptor);
  if (res != E_OK)
    return res;
#else
  struct UsbInterfaceAssociationDescriptor * const iad =
      malloc(sizeof(struct UsbInterfaceAssociationDescriptor));
  if (!iad)
    return E_MEMORY;

  iad->length = sizeof(struct UsbInterfaceAssociationDescriptor);
  iad->descriptorType = DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION;
  iad->firstInterface = usbDevCompositeIndex(driver->device, 1);
  iad->interfaceCount = 2;
  iad->functionClass = USB_CLASS_CDC;
  iad->functionSubClass = 0x02; /* Abstract Control Model */
  iad->functionProtocol = 0x00; /* None */
  iad->function = 0;

  res = usbDevAppendDescriptor(driver->device, iad);
  if (res != E_OK)
    return res;

  struct UsbInterfaceDescriptor * const compositeInterfaces =
      malloc(2 * sizeof(struct UsbInterfaceDescriptor));
  if (!compositeInterfaces)
    return E_MEMORY;

  compositeInterfaces[0].length = sizeof(struct UsbInterfaceDescriptor);
  compositeInterfaces[0].descriptorType = DESCRIPTOR_TYPE_INTERFACE;
  compositeInterfaces[0].interfaceNumber = iad->firstInterface;
  compositeInterfaces[0].alternateSettings = 0;
  compositeInterfaces[0].numEndpoints = 1;
  compositeInterfaces[0].interfaceClass = USB_CLASS_CDC;
  compositeInterfaces[0].interfaceSubClass = 0x02; //TODO
  compositeInterfaces[0].interfaceProtocol = 0x00;
  compositeInterfaces[0].interface = 0;

  compositeInterfaces[1].length = sizeof(struct UsbInterfaceDescriptor);
  compositeInterfaces[1].descriptorType = DESCRIPTOR_TYPE_INTERFACE;
  compositeInterfaces[1].interfaceNumber = iad->firstInterface + 1;
  compositeInterfaces[1].alternateSettings = 0;
  compositeInterfaces[1].numEndpoints = 2;
  compositeInterfaces[1].interfaceClass = USB_CLASS_CDC_DATA;
  compositeInterfaces[1].interfaceSubClass = 0x00;
  compositeInterfaces[1].interfaceProtocol = 0x00;
  compositeInterfaces[1].interface = 0;

  struct CdcUnionDescriptor * const unionDescriptor =
      malloc(sizeof(struct CdcUnionDescriptor));
  if (!unionDescriptor)
    return E_MEMORY;
  unionDescriptor->length = sizeof(struct CdcUnionDescriptor);
  unionDescriptor->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;
  unionDescriptor->descriptorSubType = CDC_SUBTYPE_UNION;
  unionDescriptor->masterInterface0 = iad->firstInterface;
  unionDescriptor->slaveInterface0 = iad->firstInterface + 1;
#endif

  /* Notification endpoint */
  driver->endpointDescriptors[0].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[0].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[0].endpointAddress = config->endpoint.interrupt;
  driver->endpointDescriptors[0].attributes = ENDPOINT_DESCRIPTOR_INTERRUPT;
  driver->endpointDescriptors[0].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_NOTIFICATION_EP_SIZE);
  driver->endpointDescriptors[0].interval = 8;

  /* Bulk transmit endpoint */
  driver->endpointDescriptors[1].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[1].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[1].endpointAddress = config->endpoint.tx;
  driver->endpointDescriptors[1].attributes = ENDPOINT_DESCRIPTOR_BULK;
  driver->endpointDescriptors[1].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  driver->endpointDescriptors[1].interval = 0;

  /* Bulk receive endpoint */
  driver->endpointDescriptors[2].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[2].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  driver->endpointDescriptors[2].endpointAddress = config->endpoint.rx;
  driver->endpointDescriptors[2].attributes = ENDPOINT_DESCRIPTOR_BULK;
  driver->endpointDescriptors[2].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  driver->endpointDescriptors[2].interval = 0;

  /* Append control interface descriptor */
#ifndef CONFIG_USB_COMPOSITE
  res = usbDevAppendDescriptor(driver->device, &controlInterface);
  if (res != E_OK)
    return res;
#else
  res = usbDevAppendDescriptor(driver->device, compositeInterfaces + 0);
  if (res != E_OK)
    return res;
#endif

  /* Append other control descriptors */
  for (uint8_t i = 0; i < ARRAY_SIZE(controlDescriptors); ++i)
  {
    res = usbDevAppendDescriptor(driver->device, controlDescriptors[i]);
    if (res != E_OK)
      return res;
  }

#ifdef CONFIG_USB_COMPOSITE
  res = usbDevAppendDescriptor(driver->device, unionDescriptor);
  if (res != E_OK)
    return res;
#endif

  /* Notification endpoint descriptor */
  res = usbDevAppendDescriptor(driver->device, &driver->endpointDescriptors[0]);
  if (res != E_OK)
    return res;

  /* Append data interface descriptor */
#ifndef CONFIG_USB_COMPOSITE
  res = usbDevAppendDescriptor(driver->device, &dataInterface);
  if (res != E_OK)
    return res;
#else
  res = usbDevAppendDescriptor(driver->device, compositeInterfaces + 1);
  if (res != E_OK)
    return res;
#endif

  /* Bulk transmit endpoint descriptor */
  res = usbDevAppendDescriptor(driver->device, &driver->endpointDescriptors[1]);
  if (res != E_OK)
    return res;

  /* Bulk receive endpoint descriptor */
  res = usbDevAppendDescriptor(driver->device, &driver->endpointDescriptors[2]);
  if (res != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static inline void freeDescriptors(struct CdcAcmBase *driver)
{
  for (uint8_t i = 0; i < ARRAY_SIZE(driver->endpointDescriptors); ++i)
    usbDevEraseDescriptor(driver->device, driver->endpointDescriptors + i);

  for (uint8_t i = 0; i < ARRAY_SIZE(controlDescriptors); ++i)
    usbDevEraseDescriptor(driver->device, controlDescriptors[i]);

#ifndef CONFIG_USB_COMPOSITE
  usbDevEraseDescriptor(driver->device, &dataInterface);
  usbDevEraseDescriptor(driver->device, &controlInterface);

  usbDevEraseDescriptor(driver->device, &configDescriptor);
  usbDevEraseDescriptor(driver->device, &deviceDescriptor);
#else
  //TODO
#endif
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
    {
      if (inputLength != sizeof(driver->line.coding))
        return E_VALUE;

      memcpy(&driver->line.coding, input, sizeof(driver->line.coding));
      event = true;

      usbTrace("cdc_acm: rate %u, format %u, parity %u, width %u",
          driver->line.coding.dteRate, driver->line.coding.charFormat,
          driver->line.coding.parityType, driver->line.coding.dataBits);
      break;
    }

    case CDC_GET_LINE_CODING:
    {
      if (maxOutputLength < sizeof(driver->line.coding))
        return E_VALUE;

      memcpy(output, &driver->line.coding, sizeof(driver->line.coding));
      *outputLength = sizeof(driver->line.coding);

      usbTrace("cdc_acm: line coding requested");
      break;
    }

    case CDC_SET_CONTROL_LINE_STATE:
    {
      const bool dtrState = (packet->value & 0x01) != 0;
      const bool rtsState = (packet->value & 0x02) != 0;

      driver->line.dtr = dtrState;
      driver->line.rts = rtsState;
      event = true;

      usbTrace("cdc_acm: set control lines to %02X", packet->value);
      break;
    }

    default:
      usbTrace("cdc_acm: unknown request %02X", packet->request);
      return E_INVALID;
  }

  if (event)
    cdcAcmOnParametersChanged(driver->owner);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CdcAcmBaseConfig * const config = configBase;
  struct CdcAcmBase * const driver = object;
  enum result res;

  if (!config->owner || !config->device)
    return E_VALUE;

  driver->owner = config->owner;
  driver->device = config->device;
  driver->line.coding = (struct CdcLineCoding){115200, 0, 0, 8};
  driver->line.dtr = true;
  driver->line.rts = true;

  buildDescriptors(driver, config);

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

#ifndef CONFIG_USB_COMPOSITE
  usbDevSetConnected(driver->device, true);
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CdcAcmBase * const driver = object;

#ifndef CONFIG_USB_COMPOSITE
  usbDevSetConnected(driver->device, false);
#endif

  usbDevUnbind(driver->device, driver);
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
static void driverUpdateStatus(void *object, uint8_t status)
{
  struct CdcAcmBase * const driver = object;

  cdcAcmOnStatusChanged(driver->owner, status);
}
