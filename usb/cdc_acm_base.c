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
#include <usb/composite_device.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
struct PrivateData
{
  struct UsbEndpointDescriptor endpointDescriptors[3];

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  struct UsbInterfaceAssociationDescriptor associationDescriptor;
  struct UsbInterfaceDescriptor interfaceDescriptors[2];
  struct CdcUnionDescriptor unionDescriptor;
#endif
};
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct CdcAcmBase *,
    const struct CdcAcmBaseConfig *);
static enum result descriptorEraseWrapper(void *, const void *);
static enum result iterateOverDescriptors(struct CdcAcmBase *,
    enum result (*)(void *, const void *));
static enum result handleRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CdcAcmBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcmBase = &driverTable;
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_USB_DEVICE_COMPOSITE
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

static const struct UsbInterfaceDescriptor controlInterfaceDescriptor = {
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

static const struct UsbInterfaceDescriptor dataInterfaceDescriptor = {
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

static const struct CdcUnionDescriptor unionDescriptor = {
    .length             = sizeof(struct CdcUnionDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
    .descriptorSubType  = CDC_SUBTYPE_UNION,
    .masterInterface0   = 0x00,
    .slaveInterface0    = 0x01
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
        .capabilities       = 0x00,
        .dataInterface      = 0
    },
    (const struct UsbDescriptor *)&(const struct CdcAcmDescriptor){
        .length             = sizeof(struct CdcAcmDescriptor),
        .descriptorType     = DESCRIPTOR_TYPE_CS_INTERFACE,
        .descriptorSubType  = CDC_SUBTYPE_ACM,
        .capabilities       = 0x02
    }
};
/*----------------------------------------------------------------------------*/
static enum result buildDescriptors(struct CdcAcmBase *driver,
    const struct CdcAcmBaseConfig *config)
{
  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));

  if (!privateData)
    return E_MEMORY;
  driver->privateData = privateData;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const uint8_t firstInterface = usbCompositeDevIndex(driver->device);

  driver->controlInterfaceIndex = firstInterface;

  privateData->associationDescriptor.length =
      sizeof(struct UsbInterfaceAssociationDescriptor);
  privateData->associationDescriptor.descriptorType =
      DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION;
  privateData->associationDescriptor.firstInterface = firstInterface;
  privateData->associationDescriptor.interfaceCount = 2;
  privateData->associationDescriptor.functionClass = USB_CLASS_CDC;
  /* Abstract Control Model */
  privateData->associationDescriptor.functionSubClass = 0x02;
  /* No protocol used */
  privateData->associationDescriptor.functionProtocol = 0x00;
  /* No string description */
  privateData->associationDescriptor.function = 0;

  privateData->interfaceDescriptors[0].length =
      sizeof(struct UsbInterfaceDescriptor);
  privateData->interfaceDescriptors[0].descriptorType =
      DESCRIPTOR_TYPE_INTERFACE;
  privateData->interfaceDescriptors[0].interfaceNumber = firstInterface;
  privateData->interfaceDescriptors[0].alternateSettings = 0;
  privateData->interfaceDescriptors[0].numEndpoints = 1;
  privateData->interfaceDescriptors[0].interfaceClass = USB_CLASS_CDC;
  privateData->interfaceDescriptors[0].interfaceSubClass = 0x02; //TODO
  privateData->interfaceDescriptors[0].interfaceProtocol = 0x00;
  privateData->interfaceDescriptors[0].interface = 0;

  privateData->interfaceDescriptors[1].length =
      sizeof(struct UsbInterfaceDescriptor);
  privateData->interfaceDescriptors[1].descriptorType =
      DESCRIPTOR_TYPE_INTERFACE;
  privateData->interfaceDescriptors[1].interfaceNumber = firstInterface + 1;
  privateData->interfaceDescriptors[1].alternateSettings = 0;
  privateData->interfaceDescriptors[1].numEndpoints = 2;
  privateData->interfaceDescriptors[1].interfaceClass = USB_CLASS_CDC_DATA;
  privateData->interfaceDescriptors[1].interfaceSubClass = 0x00;
  privateData->interfaceDescriptors[1].interfaceProtocol = 0x00;
  privateData->interfaceDescriptors[1].interface = 0;

  privateData->unionDescriptor.length = sizeof(struct CdcUnionDescriptor);
  privateData->unionDescriptor.descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;
  privateData->unionDescriptor.descriptorSubType = CDC_SUBTYPE_UNION;
  privateData->unionDescriptor.masterInterface0 = firstInterface;
  privateData->unionDescriptor.slaveInterface0 = firstInterface + 1;
#endif

  /* Notification endpoint */
  privateData->endpointDescriptors[0].length =
      sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptors[0].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptors[0].endpointAddress =
      config->endpoint.interrupt;
  privateData->endpointDescriptors[0].attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_INTERRUPT);
  privateData->endpointDescriptors[0].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_NOTIFICATION_EP_SIZE);
  privateData->endpointDescriptors[0].interval = 8;

  /* Bulk transmit endpoint */
  privateData->endpointDescriptors[1].length =
      sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptors[1].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptors[1].endpointAddress = config->endpoint.tx;
  privateData->endpointDescriptors[1].attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
  privateData->endpointDescriptors[1].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  privateData->endpointDescriptors[1].interval = 0;

  /* Bulk receive endpoint */
  privateData->endpointDescriptors[2].length =
      sizeof(struct UsbEndpointDescriptor);
  privateData->endpointDescriptors[2].descriptorType = DESCRIPTOR_TYPE_ENDPOINT;
  privateData->endpointDescriptors[2].endpointAddress = config->endpoint.rx;
  privateData->endpointDescriptors[2].attributes =
      ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
  privateData->endpointDescriptors[2].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  privateData->endpointDescriptors[2].interval = 0;

  return iterateOverDescriptors(driver, usbDevAppendDescriptor);
}
/*----------------------------------------------------------------------------*/
static enum result descriptorEraseWrapper(void *device, const void *descriptor)
{
  usbDevEraseDescriptor(device, descriptor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result iterateOverDescriptors(struct CdcAcmBase *driver,
    enum result (*action)(void *, const void *))
{
  struct PrivateData * const privateData = driver->privateData;

#ifdef CONFIG_USB_DEVICE_COMPOSITE
  const void * const descriptors[] = {
      &privateData->associationDescriptor,
      &privateData->interfaceDescriptors[0],
      controlDescriptors[0],
      controlDescriptors[1],
      controlDescriptors[2],
      &privateData->unionDescriptor,
      &privateData->endpointDescriptors[0],
      &privateData->interfaceDescriptors[1],
      &privateData->endpointDescriptors[1],
      &privateData->endpointDescriptors[2]
  };
#else
  const void * const descriptors[] = {
      &deviceDescriptor,
      &configDescriptor,
      &controlInterfaceDescriptor,
      controlDescriptors[0],
      controlDescriptors[1],
      controlDescriptors[2],
      &unionDescriptor,
      &privateData->endpointDescriptors[0],
      &dataInterfaceDescriptor,
      &privateData->endpointDescriptors[1],
      &privateData->endpointDescriptors[2]
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
static enum result handleRequest(struct CdcAcmBase *driver,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  bool event = false;

  switch (packet->request)
  {
    case CDC_SET_LINE_CODING:
    {
      if (payloadLength != sizeof(driver->lineCoding))
        return E_VALUE;

      memcpy(&driver->lineCoding, payload, sizeof(driver->lineCoding));
      event = true;

      usbTrace("cdc_acm at %u: rate %u, format %u, parity %u, width %u",
          packet->index, driver->lineCoding.dteRate,
          driver->lineCoding.charFormat, driver->lineCoding.parityType,
          driver->lineCoding.dataBits);
      break;
    }

    case CDC_GET_LINE_CODING:
    {
      if (maxResponseLength < sizeof(driver->lineCoding))
        return E_VALUE;

      memcpy(response, &driver->lineCoding, sizeof(driver->lineCoding));
      *responseLength = sizeof(driver->lineCoding);

      usbTrace("cdc_acm at %u: line coding requested", packet->index);
      break;
    }

    case CDC_SET_CONTROL_LINE_STATE:
    {
      driver->controlLineState = packet->value & 0x03;
      event = true;

      usbTrace("cdc_acm at %u: set control lines to %02X",
          packet->index, packet->value);
      break;
    }

    default:
    {
      usbTrace("cdc_acm at %u: unknown request %02X",
          packet->index, packet->request);
      return E_INVALID;
    }
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
  driver->lineCoding = (struct CdcLineCoding){115200, 0, 0, 8};
  driver->controlLineState = 0;
  driver->controlInterfaceIndex = 0;
  driver->speed = USB_FS;

  if ((res = buildDescriptors(driver, config)) != E_OK)
    return res;

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

#ifndef CONFIG_USB_DEVICE_COMPOSITE
  usbDevSetConnected(driver->device, true);
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CdcAcmBase * const driver = object;

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
  struct CdcAcmBase * const driver = object;
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_CLASS)
    return E_INVALID;

  if (packet->index != driver->controlInterfaceIndex)
    return E_INVALID;

  const enum result res = handleRequest(driver, packet, payload, payloadLength,
      response, responseLength, maxResponseLength);

  return res;
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct CdcAcmBase * const driver = object;

#ifdef CONFIG_USB_DEVICE_HS
  if (event == DEVICE_EVENT_PORT_CHANGE)
  {
    driver->speed = usbDevGetSpeed(driver->device);

    struct PrivateData * const privateData = driver->privateData;
    uint16_t maxPacketSize;

    if (driver->speed == USB_HS)
      maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE_HS);
    else
      maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);

    privateData->endpointDescriptors[1].maxPacketSize = maxPacketSize;
    privateData->endpointDescriptors[2].maxPacketSize = maxPacketSize;

    usbTrace("cdc_acm: current speed is %s",
        driver->speed == USB_HS ? "HS" : "FS");
  }
#endif

  cdcAcmOnEvent(driver->owner, event);
}
