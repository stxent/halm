/*
 * cdc_acm_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/cdc_acm_base.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define STRINGIFY(text) #text
#define TO_STRING(text) STRINGIFY(text)
/*----------------------------------------------------------------------------*/
static void buildDescriptors(struct CdcAcmBase *,
    const struct CdcAcmBaseConfig *);
static void freeDescriptors(struct CdcAcmBase *);
static enum result handleRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbRequest *, uint8_t *,
    uint16_t *);
static void driverDisconnect(void *);
static const struct UsbDescriptor **driverGetDescriptor(void *);
static void driverSetSuspended(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CdcAcmBase),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .disconnect = driverDisconnect,
    .getDescriptor = driverGetDescriptor,
    .setSuspended = driverSetSuspended
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcmBase = &driverTable;
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor * const deviceDescriptor =
    (const struct UsbDescriptor *)&(const struct UsbDeviceDescriptor){
    .length = sizeof(struct UsbDeviceDescriptor),
    .descriptorType = DESCRIPTOR_DEVICE,
    .usb = TO_LITTLE_ENDIAN_16(0x0101),
    .deviceClass = 0x02,
    .deviceSubClass = 0x00,
    .deviceProtocol = 0x00,
    .maxPacketSize = CDC_CONTROL_EP_SIZE,
    .idVendor = TO_LITTLE_ENDIAN_16(0xFFFF),
    .idProduct = TO_LITTLE_ENDIAN_16(0xFFFF),
    .device = TO_LITTLE_ENDIAN_16(0x0100),
    .manufacturer = 0x01,
    .product = 0x02,
    .serialNumber = 0x03,
    .numConfigurations = 0x01
};

static const struct UsbDescriptor * const configDescriptor =
    (const struct UsbDescriptor *)&(const struct UsbConfigurationDescriptor){
    .length = sizeof(struct UsbConfigurationDescriptor),
    .descriptorType = DESCRIPTOR_CONFIGURATION,
    .totalLength = TO_LITTLE_ENDIAN_16(sizeof(struct UsbConfigurationDescriptor)
        + 2 * sizeof(struct UsbInterfaceDescriptor)
        + 3 * sizeof(struct UsbEndpointDescriptor)
        + sizeof(struct CdcHeaderDescriptor)
        + sizeof(struct CdcCallManagementDescriptor)
        + sizeof(struct CdcAcmDescriptor)
        + sizeof(struct CdcUnionDescriptor)),
    .numInterfaces = 0x02,
    .configurationValue = 0x01,
    .configuration = 0x00,
    .attributes = 0xC0,
    .maxPower = ((CONFIG_USB_DEVICE_CURRENT + 1) >> 1)
};

static const struct UsbDescriptor * const controlDescriptors[] = {
    (const struct UsbDescriptor *)&(const struct UsbInterfaceDescriptor){
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_INTERFACE,
        .interfaceNumber = 0x00,
        .alternateSettings = 0x00,
        .numEndpoints = 0x01,
        .interfaceClass = 0x02,
        .interfaceSubClass = 0x02,
        .interfaceProtocol = 0x01,
        .interface = 0x00
    },
    (const struct UsbDescriptor *)&(const struct CdcHeaderDescriptor){
        .length = sizeof(struct CdcHeaderDescriptor),
        .descriptorType = 0x24,
        .descriptorSubType = 0x00,
        .cdc = TO_LITTLE_ENDIAN_16(0x0110)
    },
    (const struct UsbDescriptor *)&(const struct CdcCallManagementDescriptor){
        .length = sizeof(struct CdcCallManagementDescriptor),
        .descriptorType = 0x24,
        .descriptorSubType = 0x01,
        .capabilities = 0x01,
        .dataInterface = 0x01
    },
    (const struct UsbDescriptor *)&(const struct CdcAcmDescriptor){
        .length = sizeof(struct CdcAcmDescriptor),
        .descriptorType = 0x24,
        .descriptorSubType = 0x02,
        .capabilities = 0x02
    },
    (const struct UsbDescriptor *)&(const struct CdcUnionDescriptor){
        .length = sizeof(struct CdcUnionDescriptor),
        .descriptorType = 0x24,
        .descriptorSubType = 0x06,
        .masterInterface0 = 0x00,
        .slaveInterface0 = 0x01
    }
};

static const struct UsbDescriptor * const dataDescriptors[] = {
    (const struct UsbDescriptor *)&(const struct UsbInterfaceDescriptor){
        .length = sizeof(struct UsbInterfaceDescriptor),
        .descriptorType = DESCRIPTOR_INTERFACE,
        .interfaceNumber = 0x01,
        .alternateSettings = 0x00,
        .numEndpoints = 0x02,
        .interfaceClass = 0x0A,
        .interfaceSubClass = 0x00,
        .interfaceProtocol = 0x00,
        .interface = 0x00
    }
};

static const struct UsbDescriptor * const stringDescriptors[] = {
    (const struct UsbDescriptor *)&(const struct UsbStringHeadDescriptor){
        .length = sizeof(struct UsbStringHeadDescriptor),
        .descriptorType = DESCRIPTOR_STRING,
        .langid = TO_LITTLE_ENDIAN_16(0x0409)
    },
    (const struct UsbDescriptor *)&(const struct UsbStringDescriptor){
        .length = 0,
        .descriptorType = DESCRIPTOR_STRING,
        .data = TO_STRING(CONFIG_USB_DEVICE_VENDOR_NAME)
    },
    (const struct UsbDescriptor *)&(const struct UsbStringDescriptor){
        .length = 0,
        .descriptorType = DESCRIPTOR_STRING,
        .data = TO_STRING(CONFIG_USB_DEVICE_PRODUCT_NAME)
    },
    (const struct UsbDescriptor *)&(const struct UsbStringDescriptor){
        .length = 0,
        .descriptorType = DESCRIPTOR_STRING,
        .data = TO_STRING(CONFIG_USB_DEVICE_SERIAL)
    }
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

  driver->descriptorArray[index++] = deviceDescriptor;
  driver->descriptorArray[index++] = configDescriptor;

  /* Copy control interface descriptors */
  for (uint8_t i = 0; i < ARRAY_SIZE(controlDescriptors); ++i)
    driver->descriptorArray[index++] = controlDescriptors[i];

  /* Notification endpoint */
  driver->endpointDescriptors[0].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[0].descriptorType = DESCRIPTOR_ENDPOINT;
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
  driver->endpointDescriptors[1].descriptorType = DESCRIPTOR_ENDPOINT;
  driver->endpointDescriptors[1].endpointAddress = config->endpoint.tx;
  driver->endpointDescriptors[1].attributes = 0x02;
  driver->endpointDescriptors[1].maxPacketSize =
      TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE);
  driver->endpointDescriptors[1].interval = 0;

  /* Bulk receive endpoint */
  driver->endpointDescriptors[2].length = sizeof(struct UsbEndpointDescriptor);
  driver->endpointDescriptors[2].descriptorType = DESCRIPTOR_ENDPOINT;
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
    const uint8_t serialIndex =
        ((const struct UsbDeviceDescriptor *)deviceDescriptor)->serialNumber;

    driver->stringDescriptors = malloc(sizeof(struct UsbStringDescriptor));
    driver->stringDescriptors[0].length = 0;
    driver->stringDescriptors[0].descriptorType = DESCRIPTOR_STRING;
    driver->stringDescriptors[0].data = config->serial; //TODO Allocate memory?

    driver->descriptorArray[index + serialIndex] =
        (struct UsbDescriptor *)&driver->stringDescriptors[0];
  }
  else
  {
    driver->stringDescriptors = 0;
  }
  index += ARRAY_SIZE(stringDescriptors);

  /* Add end of the array mark */
  driver->descriptorArray[index] = 0;
}
/*----------------------------------------------------------------------------*/
static void freeDescriptors(struct CdcAcmBase *driver)
{
  if (driver->stringDescriptors)
    free(driver->stringDescriptors);
  free(driver->endpointDescriptors);
  free(driver->descriptorArray);
}
/*----------------------------------------------------------------------------*/
static enum result handleRequest(struct CdcAcmBase *driver,
    const struct UsbSetupPacket *packet, const uint8_t *input,
    uint16_t inputLength, uint8_t *output, uint16_t *outputLength)
{
  switch (packet->request)
  {
    case CDC_SET_LINE_CODING:
      memcpy(&driver->lineCoding, input, sizeof(driver->lineCoding));
      *outputLength = 0;
      usbTrace("cdc_acm: rate %u, format %u, parity %u, width %u",
          driver->lineCoding.dteRate, driver->lineCoding.charFormat,
          driver->lineCoding.parityType, driver->lineCoding.dataBits);
      break;

    case CDC_GET_LINE_CODING:
      memcpy(output, &driver->lineCoding, sizeof(driver->lineCoding));
      *outputLength = sizeof(driver->lineCoding);
      usbTrace("cdc_acm: line coding requested");
      break;

    case CDC_SET_CONTROL_LINE_STATE:
      // FIXME bit0 = DTR, bitWhat = RTS
      *outputLength = 0;
      usbTrace("cdc_acm: set control lines to %02X", packet->value);
      break;

    default:
      usbTrace("cdc_acm: unknown request %02X", packet->request);
      return E_INVALID;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CdcAcmBaseConfig * const config = configBase;
  struct CdcAcmBase * const driver = object;
  enum result res;

  driver->device = config->device;
  driver->buffer = malloc(64); //FIXME Magic

  driver->lineCoding = (struct CdcLineCoding){115200, 0, 0, 8};

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
  freeDescriptors(driver);
  free(driver->buffer);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbRequest *request, uint8_t *reply, uint16_t *length)
{
  struct CdcAcmBase * const driver = object;
  struct UsbSetupPacket * const packet = &driver->setupPacket;

  if (request->status & REQUEST_SETUP)
  {
    memcpy(packet, request->buffer, sizeof(struct UsbSetupPacket));
    packet->value = fromLittleEndian16(packet->value);
    packet->index = fromLittleEndian16(packet->index);
    packet->length = fromLittleEndian16(packet->length);

    const uint8_t direction = REQUEST_DIRECTION_VALUE(packet->requestType);
    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_CLASS)
      return E_INVALID;

    driver->dataLength = packet->length;

    if (!packet->length || direction == REQUEST_DIRECTION_TO_HOST)
    {
      return handleRequest(driver, packet, 0, 0, reply, length);
    }
    else
    {
      *length = 0;
      return E_OK;
    }
  }
  else
  {
    const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

    if (type != REQUEST_TYPE_CLASS)
      return E_INVALID;

    if (driver->dataLength)
    {
      //FIXME Rewrite
      memcpy(driver->buffer, request->buffer, request->length);

      if (driver->dataLength == request->length)
      {
        return handleRequest(driver, packet, driver->buffer,
            driver->dataLength, reply, length);
      }
    }
    else
    {
      *length = 0;
      return E_OK;
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void driverDisconnect(void *object)
{
  struct CdcAcmBase * const driver = object;
}
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor **driverGetDescriptor(void *object)
{
  const struct CdcAcmBase * const driver = object;

  return driver->descriptorArray;
}
/*----------------------------------------------------------------------------*/
static void driverSetSuspended(void *object, bool state)
{
  struct CdcAcmBase * const driver = object;
}
