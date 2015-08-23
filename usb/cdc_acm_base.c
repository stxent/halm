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
//FIXME Move to other file
#define INT_IN_EP   0x81
#define BULK_OUT_EP   0x05
#define BULK_IN_EP    0x82

#define MAX_PACKET_SIZE 64
#define MAX_DATA_SIZE  64
/*----------------------------------------------------------------------------*/
static enum result handleRequest(struct CdcAcmBase *,
    const struct UsbSetupPacket *, const uint8_t *, uint16_t, uint8_t *,
    uint16_t *);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbRequest *,
    uint8_t *, uint16_t *);
static void driverDisconnect(void *);
static const struct UsbDescriptor *driverGetDescriptor(void *);
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
//FIXME Endianness for totalLength
static const struct UsbDescriptor rootDescriptor = {
    .payload = 0,
    .count = 3,
    .children = (const struct UsbDescriptor []){
        {
            .payload = (const struct UsbDescriptorHeader *)
                (const void *)&(const struct UsbDeviceDescriptor){
                .length = sizeof(struct UsbDeviceDescriptor),
                .descriptorType = DESCRIPTOR_DEVICE,
                .usb = 0x0101, //FIXME
                .deviceClass = 0x02,
                .deviceSubClass = 0x00,
                .deviceProtocol = 0x00,
                .maxPacketSize = MAX_PACKET_SIZE,
                .idVendor = 0xFFFF, //FIXME
                .idProduct = 0x0005, //FIXME
                .device = 0x0100, //FIXME
                .manufacturer = 0x01,
                .product = 0x02,
                .serialNumber = 0x03,
                .numConfigurations = 0x01
            },
            .count = 0,
            .children = 0
        },
        {
            .payload = (const struct UsbDescriptorHeader *)
                &(const struct UsbConfigurationDescriptor){
                .length = sizeof(struct UsbConfigurationDescriptor),
                .descriptorType = DESCRIPTOR_CONFIGURATION,
                .totalLength = sizeof(struct UsbConfigurationDescriptor)
                    + 2 * sizeof(struct UsbInterfaceDescriptor)
                    + 3 * sizeof(struct UsbEndpointDescriptor)
                    + sizeof(struct CdcHeaderDescriptor)
                    + sizeof(struct CdcCallManagementDescriptor)
                    + sizeof(struct CdcAcmDescriptor)
                    + sizeof(struct CdcUnionDescriptor),
                .numInterfaces = 0x02,
                .configurationValue = 0x01,
                .configuration = 0x00,
                .attributes = 0xC0,
                .maxPower = 0x32 //FIXME
            },
            .count = 9,
            .children = (const struct UsbDescriptor []){
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbInterfaceDescriptor){
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
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct CdcHeaderDescriptor){
                        .length = sizeof(struct CdcHeaderDescriptor),
                        .descriptorType = 0x24,
                        .descriptorSubType = 0x00,
                        .cdc = 0x0110 //FIXME
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct CdcCallManagementDescriptor){
                        .length =
                            sizeof(struct CdcCallManagementDescriptor),
                        .descriptorType = 0x24,
                        .descriptorSubType = 0x01,
                        .capabilities = 0x01,
                        .dataInterface = 0x01
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct CdcAcmDescriptor){
                        .length = sizeof(struct CdcAcmDescriptor),
                        .descriptorType = 0x24,
                        .descriptorSubType = 0x02,
                        .capabilities = 0x02
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct CdcUnionDescriptor){
                        .length = sizeof(struct CdcUnionDescriptor),
                        .descriptorType = 0x24,
                        .descriptorSubType = 0x06,
                        .masterInterface0 = 0x00,
                        .slaveInterface0 = 0x01
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbEndpointDescriptor){
                        .length = sizeof(struct UsbEndpointDescriptor),
                        .descriptorType = DESCRIPTOR_ENDPOINT,
                        .endpointAddress = INT_IN_EP,
                        .attributes = 0x03, //FIXME
                        .maxPacketSize = 10, //FIXME LE
                        .interval = 0x08
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbInterfaceDescriptor){
                        .length = sizeof(struct UsbInterfaceDescriptor),
                        .descriptorType = DESCRIPTOR_INTERFACE,
                        .interfaceNumber = 0x01,
                        .alternateSettings = 0x00,
                        .numEndpoints = 0x02,
                        .interfaceClass = 0x0A,
                        .interfaceSubClass = 0x00,
                        .interfaceProtocol = 0x00,
                        .interface = 0x00
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbEndpointDescriptor){
                        .length = sizeof(struct UsbEndpointDescriptor),
                        .descriptorType = DESCRIPTOR_ENDPOINT,
                        .endpointAddress = BULK_OUT_EP,
                        .attributes = 0x02,
                        .maxPacketSize = MAX_DATA_SIZE, //FIXME
                        .interval = 0x00
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbEndpointDescriptor){
                        .length = sizeof(struct UsbEndpointDescriptor),
                        .descriptorType = DESCRIPTOR_ENDPOINT,
                        .endpointAddress = BULK_IN_EP,
                        .attributes = 0x02,
                        .maxPacketSize = MAX_DATA_SIZE, //FIXME
                        .interval = 0x00
                    },
                    .count = 0,
                    .children = 0
                }
            }
        },
        {
            .payload = (const struct UsbDescriptorHeader *)
                (const void *)&(const struct UsbStringHeadDescriptor){
                .length = sizeof(struct UsbStringHeadDescriptor),
                .descriptorType = DESCRIPTOR_STRING,
                .langid = 0x0409 //FIXME
            },
            .count = 3,
            .children = (const struct UsbDescriptor []){
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 6,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "LPCUSB" //FIXME
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 9,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "USBSerial" //FIXME
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 8,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "DEADBEEF" //FIXME
                    },
                    .count = 0,
                    .children = 0
                }
            }
        }
    }
};
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
static const struct UsbDescriptor *driverGetDescriptor(void *object
    __attribute__((unused)))
{
  return &rootDescriptor;
}
/*----------------------------------------------------------------------------*/
static void driverSetSuspended(void *object, bool state)
{
  struct CdcAcmBase * const driver = object;
}
