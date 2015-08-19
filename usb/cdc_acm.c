/*
 * cdc_acm.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/cdc_acm.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
//FIXME Move to other file
#define VCOM_FIFO_SIZE  128

#define INT_IN_EP   0x81
#define BULK_OUT_EP   0x05
#define BULK_IN_EP    0x82

#define MAX_PACKET_SIZE 64
#define MAX_DATA_SIZE  64

#define SET_LINE_CODING     0x20
#define GET_LINE_CODING     0x21
#define SET_CONTROL_LINE_STATE  0x22
/*----------------------------------------------------------------------------*/
//static void driverTransferHandler(struct UsbRequest *, void *);
static enum result handleRequest(struct CdcAcm *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *);
static void cdcDataReceived(struct UsbRequest *, void *);
static void cdcDataSent(struct UsbRequest *, void *);
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
    .size = sizeof(struct CdcAcm),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .disconnect = driverDisconnect,
    .getDescriptor = driverGetDescriptor,
    .setSuspended = driverSetSuspended
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CdcAcm = &driverTable;
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor rootDescriptor = {
    .payload = 0,
    .count = 3,
    .children = (const struct UsbDescriptor []){
        {
            .payload = (const struct UsbDescriptorHeader *)
                (const void *)&(const struct UsbDeviceDescriptor){
                .length = sizeof(struct UsbDeviceDescriptor),
                .descriptorType = DESCRIPTOR_DEVICE,
                .usb = 0x0101,
                .deviceClass = 0x02,
                .deviceSubClass = 0x00,
                .deviceProtocol = 0x00,
                .maxPacketSize = MAX_PACKET_SIZE,
                .idVendor = 0xFFFF,
                .idProduct = 0x0005,
                .device = 0x0100,
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
                .maxPower = 0x32
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
                        .descriptorType = 0x24, //FIXME
                        .descriptorSubType = 0x00,
                        .cdc = 0x0110
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct CdcCallManagementDescriptor){
                        .length =
                            sizeof(struct CdcCallManagementDescriptor),
                        .descriptorType = 0x24, //FIXME
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
                        .descriptorType = 0x24, //FIXME
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
                        .descriptorType = 0x24, //FIXME
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
                        .maxPacketSize = MAX_DATA_SIZE,
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
                        .maxPacketSize = MAX_DATA_SIZE,
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
                .langid = 0x0409
            },
            .count = 3,
            .children = (const struct UsbDescriptor []){
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 6,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "LPCUSB"
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 9,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "USBSerial"
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 8,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "DEADBEEF"
                    },
                    .count = 0,
                    .children = 0
                }
            }
        }
    }
};
/*----------------------------------------------------------------------------*/
static enum result handleRequest(struct CdcAcm *driver,
    const struct UsbSetupPacket *packet, const uint8_t *input,
    uint16_t inputLength, uint8_t *output, uint16_t *outputLength)
{
  switch (packet->request)
  {
    case SET_LINE_CODING:
      memcpy(&driver->lineCoding, input, sizeof(driver->lineCoding));
      *outputLength = 0;
      usbTrace("cdc_acm: rate %u, format %u, parity %u, width %u",
          driver->lineCoding.dteRate, driver->lineCoding.charFormat,
          driver->lineCoding.parityType, driver->lineCoding.dataBits);
      break;

    case GET_LINE_CODING:
      memcpy(output, &driver->lineCoding, sizeof(driver->lineCoding));
      *outputLength = sizeof(driver->lineCoding);
      usbTrace("cdc_acm: line coding requested");
      break;

    case SET_CONTROL_LINE_STATE:
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
static void cdcDataReceived(struct UsbRequest *request, void *argument)
{
  struct CdcAcm * const driver = argument;

//  pinSet(led);
  byteQueuePushArray(&driver->rxfifo, request->buffer, request->length);

  request->length = 0;
  request->status = 0;
  usbEpEnqueue(driver->inputDataEp, request);
//  pinReset(led);

  //XXX ONLY FOR TEST
  while (!byteQueueEmpty(&driver->rxfifo)
      && !queueEmpty(&driver->outputDataReqs))
  {
    struct UsbRequest *req;
    queuePop(&driver->outputDataReqs, &req);
    const unsigned int length = byteQueuePopArray(&driver->rxfifo, req->buffer,
        req->capacity);
    req->length = length;
    req->status = 0;

    usbEpEnqueue(driver->outputDataEp, req);
  }
}
/*----------------------------------------------------------------------------*/
static void cdcDataSent(struct UsbRequest *request, void *argument)
{
  struct CdcAcm * const driver = argument;

  queuePush(&driver->outputDataReqs, &request);
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CdcAcmConfig * const config = configBase;
  struct CdcAcm * const driver = object;
  enum result res;

  driver->device = config->device;
  driver->buffer = malloc(64); //FIXME Magic
  //TODO Add function for callback setup

  driver->lineCoding = (struct CdcLineCoding){115200, 0, 0, 8};

  //TODO Add TX FIFO
  if ((res = byteQueueInit(&driver->rxfifo, VCOM_FIFO_SIZE)) != E_OK)
    return res;

  //TODO Add rollback
  driver->intEp = usbDevAllocate(driver->device, 10, INT_IN_EP);
  if (!driver->intEp)
    return E_ERROR;
  driver->inputDataEp = usbDevAllocate(driver->device, MAX_DATA_SIZE,
      BULK_OUT_EP);
  if (!driver->inputDataEp)
    return E_ERROR;
  driver->outputDataEp = usbDevAllocate(driver->device, MAX_DATA_SIZE,
      BULK_IN_EP);
  if (!driver->outputDataEp)
    return E_ERROR;

  //TODO Rewrite allocation
  for (unsigned int i = 0; i < 2; ++i)
  {
    struct UsbRequest * const request = malloc(sizeof(struct UsbRequest));
    usbRequestInit(request, MAX_DATA_SIZE);

    request->callback = cdcDataReceived;
    request->callbackArgument = driver;
    usbEpEnqueue(driver->inputDataEp, request);
  }

  queueInit(&driver->outputDataReqs, sizeof(struct UsbRequest *), 2);
  for (unsigned int i = 0; i < 2; ++i)
  {
    struct UsbRequest * const request = malloc(sizeof(struct UsbRequest));
    usbRequestInit(request, MAX_DATA_SIZE);

    request->callback = cdcDataSent;
    request->callbackArgument = driver;
    queuePush(&driver->outputDataReqs, &request);
  }

  if ((res = usbDevBind(driver->device, driver)) != E_OK)
    return res;

  usbDevSetConnected(driver->device, true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CdcAcm * const driver = object;

  usbDevSetConnected(driver->device, false);
  //TODO Free requests
  free(driver->buffer);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbRequest *request, uint8_t *reply, uint16_t *length)
{
  struct CdcAcm * const driver = object;
  struct UsbSetupPacket * const packet = &driver->setupPacket;
  enum result res;

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
  struct CdcAcm * const driver = object;
}
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor *driverGetDescriptor(void *object)
{
  struct CdcAcm * const driver = object;

  return &rootDescriptor;
}
/*----------------------------------------------------------------------------*/
static void driverSetSuspended(void *object, bool state)
{
  struct CdcAcm * const driver = object;
}
