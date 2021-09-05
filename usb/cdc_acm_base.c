/*
 * cdc_acm_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/cdc_acm.h>
#include <halm/usb/cdc_acm_base.h>
#include <halm/usb/cdc_acm_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
struct PrivateData
{
  /* Line settings */
  struct
  {
    struct CdcLineCoding lineCoding;
    uint8_t controlLineState;
  } state;
};
/*----------------------------------------------------------------------------*/
static void interfaceAssociationDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
static void controlInterfaceDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void cdcHeaderDescriptor(const void *, struct UsbDescriptor *, void *);
static void cdcCallManagementDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void cdcAcmDescriptor(const void *, struct UsbDescriptor *, void *);
static void unionDescriptor(const void *, struct UsbDescriptor *, void *);
static void notificationEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void dataInterfaceDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void dataEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void bulkReceiveEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
static void bulkTransmitEndpointDescriptor(const void *, struct UsbDescriptor *,
    void *);
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, void *, uint16_t *);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcmBase = &(const struct UsbDriverClass){
    .size = sizeof(struct CdcAcmBase),
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

    interfaceAssociationDescriptor,

    controlInterfaceDescriptor,
    cdcHeaderDescriptor,
    cdcCallManagementDescriptor,
    cdcAcmDescriptor,
    unionDescriptor,
    notificationEndpointDescriptor,
    dataInterfaceDescriptor,
    bulkReceiveEndpointDescriptor,
    bulkTransmitEndpointDescriptor,
    0
};
/*----------------------------------------------------------------------------*/
static void interfaceAssociationDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceAssociationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION;

  if (payload)
  {
    struct UsbInterfaceAssociationDescriptor * const descriptor = payload;

    descriptor->firstInterface = driver->controlInterfaceIndex;
    descriptor->interfaceCount = 2;
    descriptor->functionClass = USB_CLASS_CDC;
    descriptor->functionSubClass = 0x02; /* Abstract Control Model */
  }
}
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
    descriptor->deviceClass = USB_CLASS_CDC;
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_CONTROL_EP_SIZE);
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
        + sizeof(struct UsbInterfaceDescriptor) * 2
        + sizeof(struct UsbEndpointDescriptor) * 3
        + sizeof(struct CdcHeaderDescriptor)
        + sizeof(struct CdcCallManagementDescriptor)
        + sizeof(struct CdcAcmDescriptor)
        + sizeof(struct CdcUnionDescriptor));
    descriptor->numInterfaces = 2;
    descriptor->configurationValue = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void controlInterfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    struct UsbInterfaceDescriptor * const descriptor = payload;

    descriptor->interfaceNumber = driver->controlInterfaceIndex;
    descriptor->numEndpoints = 1;
    descriptor->interfaceClass = USB_CLASS_CDC;
    descriptor->interfaceSubClass = 0x02; /* Abstract Control Model */
  }
}
/*----------------------------------------------------------------------------*/
static void cdcHeaderDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct CdcHeaderDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    struct CdcHeaderDescriptor * const descriptor = payload;

    descriptor->descriptorSubType = CDC_SUBTYPE_HEADER;
    descriptor->cdc = TO_LITTLE_ENDIAN_16(0x0110);
  }
}
/*----------------------------------------------------------------------------*/
static void cdcCallManagementDescriptor(const void *object
    __attribute__((unused)), struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct CdcCallManagementDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    struct CdcCallManagementDescriptor * const descriptor = payload;

    descriptor->descriptorSubType = CDC_SUBTYPE_CALL_MANAGEMENT;
  }
}
/*----------------------------------------------------------------------------*/
static void cdcAcmDescriptor(const void *object __attribute__((unused)),
    struct UsbDescriptor *header, void *payload)
{
  header->length = sizeof(struct CdcAcmDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    struct CdcAcmDescriptor * const descriptor = payload;

    descriptor->descriptorSubType = CDC_SUBTYPE_ACM;
    descriptor->capabilities = 0x02;
  }
}
/*----------------------------------------------------------------------------*/
static void unionDescriptor(const void *object, struct UsbDescriptor *header,
    void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct CdcUnionDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CS_INTERFACE;

  if (payload)
  {
    struct CdcUnionDescriptor * const descriptor = payload;

    descriptor->descriptorSubType = CDC_SUBTYPE_UNION;
    descriptor->masterInterface0 = driver->controlInterfaceIndex;
    descriptor->slaveInterface0 = driver->controlInterfaceIndex + 1;
  }
}
/*----------------------------------------------------------------------------*/
static void notificationEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpoints.interrupt;
    descriptor->attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_INTERRUPT);
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_NOTIFICATION_EP_SIZE);
    descriptor->interval = 8;
  }
}
/*----------------------------------------------------------------------------*/
static void dataInterfaceDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct UsbInterfaceDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_INTERFACE;

  if (payload)
  {
    struct UsbInterfaceDescriptor * const descriptor = payload;

    descriptor->interfaceNumber = driver->controlInterfaceIndex + 1;
    descriptor->numEndpoints = 2;
    descriptor->interfaceClass = USB_CLASS_CDC_DATA;
  }
}
/*----------------------------------------------------------------------------*/
static void dataEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  const struct CdcAcmBase * const driver = object;

  header->length = sizeof(struct UsbEndpointDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_ENDPOINT;

  if (payload)
  {
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->attributes = ENDPOINT_DESCRIPTOR_TYPE(ENDPOINT_TYPE_BULK);
    descriptor->interval = 0;

    if (driver->speed == USB_HS)
      descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE_HS);
    else
      descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  }
}
/*----------------------------------------------------------------------------*/
static void bulkTransmitEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  dataEndpointDescriptor(object, header, payload);

  if (payload)
  {
    const struct CdcAcmBase * const driver = object;
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpoints.tx;
  }
}
/*----------------------------------------------------------------------------*/
static void bulkReceiveEndpointDescriptor(const void *object,
    struct UsbDescriptor *header, void *payload)
{
  dataEndpointDescriptor(object, header, payload);

  if (payload)
  {
    const struct CdcAcmBase * const driver = object;
    struct UsbEndpointDescriptor * const descriptor = payload;

    descriptor->endpointAddress = driver->endpoints.rx;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result handleClassRequest(struct CdcAcmBase *driver,
    const struct UsbSetupPacket *packet, void *response,
    uint16_t *responseLength)
{
  if (packet->index != driver->controlInterfaceIndex)
    return E_INVALID;

  struct PrivateData * const privateData = driver->privateData;
  enum Result res = E_OK;
  bool event = false;

  switch (packet->request)
  {
    case CDC_SET_LINE_CODING:
    {
      if (packet->length == sizeof(struct CdcLineCoding))
      {
        struct CdcLineCoding * const settings = &privateData->state.lineCoding;

        memcpy(settings, response, sizeof(*settings));
        settings->dteRate = fromLittleEndian32(settings->dteRate);

        event = true;

        usbTrace("cdc_acm at %u: rate %u, format %u, parity %u, width %u",
            driver->controlInterfaceIndex, settings->dteRate,
            settings->charFormat, settings->parityType, settings->dataBits);
      }
      else
      {
        res = E_VALUE; /* Incorrect packet */
      }

      break;
    }

    case CDC_GET_LINE_CODING:
    {
      struct CdcLineCoding * const settings = response;

      memcpy(settings, &privateData->state.lineCoding, sizeof(*settings));
      settings->dteRate = toLittleEndian32(settings->dteRate);
      *responseLength = sizeof(*settings);

      usbTrace("cdc_acm at %u: line coding requested",
          driver->controlInterfaceIndex);
      break;
    }

    case CDC_SET_CONTROL_LINE_STATE:
    {
      privateData->state.controlLineState = packet->value & 0x03;
      event = true;

      usbTrace("cdc_acm at %u: set control lines to %02X",
          driver->controlInterfaceIndex, packet->value);
      break;
    }

    default:
      usbTrace("cdc_acm at %u: unknown request %02X",
          driver->controlInterfaceIndex, packet->request);
      res = E_INVALID;
      break;
  }

  if (event)
    cdcAcmOnParametersChanged(driver->owner);

  return res;
}
/*----------------------------------------------------------------------------*/
uint32_t cdcAcmBaseGetRate(const struct CdcAcmBase *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  return privateData->state.lineCoding.dteRate;
}
/*----------------------------------------------------------------------------*/
uint8_t cdcAcmBaseGetState(const struct CdcAcmBase *driver)
{
  const struct PrivateData * const privateData = driver->privateData;
  return privateData->state.controlLineState;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
{
  const struct CdcAcmBaseConfig * const config = configBase;
  assert(config->owner);
  assert(config->device);

  struct CdcAcmBase * const driver = object;

  driver->owner = config->owner;
  driver->endpoints.interrupt = config->endpoints.interrupt;
  driver->endpoints.tx = config->endpoints.tx;
  driver->endpoints.rx = config->endpoints.rx;
  driver->device = config->device;
  driver->speed = USB_FS;

  struct PrivateData * const privateData = malloc(sizeof(struct PrivateData));
  if (!privateData)
    return E_MEMORY;

  privateData->state.lineCoding = (struct CdcLineCoding){115200, 0, 0, 8};
  privateData->state.controlLineState = 0;
  driver->privateData = privateData;

  driver->controlInterfaceIndex = usbDevGetInterface(driver->device);
  return usbDevBind(driver->device, driver);
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CdcAcmBase * const driver = object;

  usbDevUnbind(driver->device, driver);
  free(driver->privateData);
}
/*----------------------------------------------------------------------------*/
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer, uint16_t *responseLength,
    uint16_t maxResponseLength __attribute__((unused)))
{
  if (REQUEST_TYPE_VALUE(packet->requestType) == REQUEST_TYPE_CLASS)
    return handleClassRequest(object, packet, buffer, responseLength);
  else
    return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
{
  return deviceDescriptorTable;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct CdcAcmBase * const driver = object;

#ifdef CONFIG_USB_DEVICE_HS
  if (event == USB_DEVICE_EVENT_PORT_CHANGE)
  {
    driver->speed = usbDevGetSpeed(driver->device);

    usbTrace("cdc_acm: current speed is %s",
        driver->speed == USB_HS ? "HS" : "FS");
  }
#endif

  switch ((enum UsbDeviceEvent)event)
  {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_SUSPEND:
    case USB_DEVICE_EVENT_RESUME:
      cdcAcmOnEvent(driver->owner, event);
      break;

    default:
      break;
  }
}
