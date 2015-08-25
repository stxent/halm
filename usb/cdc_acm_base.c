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
static const void *buildDescriptors(const struct CdcAcmBaseConfig *);
static void freeDescriptors(const struct UsbDescriptor *);
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
static const struct UsbDescriptor rootDescriptor = {
    .payload = 0,
    .count = 3,
    .children = (const struct UsbDescriptor []){
        {
            .payload = (const struct UsbDescriptorHeader *)
                &(const struct UsbDeviceDescriptor){
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
            },
            .count = 0,
            .children = 0
        },
        {
            .payload = (const struct UsbDescriptorHeader *)
                &(const struct UsbConfigurationDescriptor){
                .length = sizeof(struct UsbConfigurationDescriptor),
                .descriptorType = DESCRIPTOR_CONFIGURATION,
                .totalLength = TO_LITTLE_ENDIAN_16(
                    sizeof(struct UsbConfigurationDescriptor)
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
                .maxPower = (CONFIG_USB_CURRENT >> 1)
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
                        .cdc = TO_LITTLE_ENDIAN_16(0x0110)
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
                        .endpointAddress = 0xFF,
                        .attributes = 0x03,
                        .maxPacketSize =
                            TO_LITTLE_ENDIAN_16(CDC_NOTIFICATION_EP_SIZE),
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
                        .endpointAddress = 0xFF,
                        .attributes = 0x02,
                        .maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE),
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
                        .endpointAddress = 0xFF,
                        .attributes = 0x02,
                        .maxPacketSize = TO_LITTLE_ENDIAN_16(CDC_DATA_EP_SIZE),
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
                .langid = TO_LITTLE_ENDIAN_16(0x0409)
            },
            .count = 3,
            .children = (const struct UsbDescriptor []){
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 7,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "LPC USB"
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 10,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "USB Serial"
                    },
                    .count = 0,
                    .children = 0
                },
                {
                    .payload = (const struct UsbDescriptorHeader *)
                        &(const struct UsbStringDescriptor){
                        .length = 2 + 10,
                        .descriptorType = DESCRIPTOR_STRING,
                        .data = "1234567890"
                    },
                    .count = 0,
                    .children = 0
                }
            }
        }
    }
};
/*----------------------------------------------------------------------------*/
static const void *buildDescriptors(const struct CdcAcmBaseConfig *config)
{
  struct UsbDescriptor * const root = malloc(sizeof(struct UsbDescriptor));

  root->children = malloc(3 * sizeof(struct UsbDescriptor));
  root->count = 3;
  root->payload = 0;

  memcpy((void *)root->children, rootDescriptor.children,
      3 * sizeof(struct UsbDescriptor));

  /* Create device descriptor */
  if (config->product && config->vendor)
  {
    struct UsbDeviceDescriptor * const deviceDescriptor =
        malloc(sizeof(struct UsbDeviceDescriptor));

    memcpy(deviceDescriptor, rootDescriptor.children[0].payload,
        sizeof(struct UsbDeviceDescriptor));
    if (config->product)
      deviceDescriptor->idProduct = config->product;
    if (config->vendor)
      deviceDescriptor->idVendor = config->vendor;

    ((struct UsbDescriptor *)root->children)[0].payload =
        (struct UsbDescriptorHeader *)deviceDescriptor;
  }

  /* Create endpoint descriptors */
  struct UsbEndpointDescriptor * const endpoints =
      malloc(3 * sizeof(struct UsbEndpointDescriptor));

  memcpy(&endpoints[0], rootDescriptor.children[1].children[5].payload,
      sizeof(struct UsbEndpointDescriptor));
  memcpy(&endpoints[1], rootDescriptor.children[1].children[7].payload,
      sizeof(struct UsbEndpointDescriptor));
  memcpy(&endpoints[2], rootDescriptor.children[1].children[8].payload,
      sizeof(struct UsbEndpointDescriptor));

  endpoints[0].endpointAddress = config->endpoint.interrupt;
  endpoints[1].endpointAddress = config->endpoint.tx;
  endpoints[2].endpointAddress = config->endpoint.rx;

  ((struct UsbDescriptor *)root->children)[1].children =
      malloc(9 * sizeof(struct UsbDescriptor));
  memcpy((void *)root->children[1].children,
      rootDescriptor.children[1].children, 9 * sizeof(struct UsbDescriptor));

  ((struct UsbDescriptor *)root->children[1].children)[5].payload =
      (struct UsbDescriptorHeader *)&endpoints[0];
  ((struct UsbDescriptor *)root->children[1].children)[7].payload =
      (struct UsbDescriptorHeader *)&endpoints[1];
  ((struct UsbDescriptor *)root->children[1].children)[8].payload =
      (struct UsbDescriptorHeader *)&endpoints[2];

  /* String descriptors */
  if (config->productString || config->serialString || config->vendorString)
  {
    ((struct UsbDescriptor *)root->children)[2].children =
        malloc(3 * sizeof(struct UsbDescriptor));

    memcpy((void *)root->children[2].children,
        rootDescriptor.children[2].children, 3 * sizeof(struct UsbDescriptor));

    /* Serial number */
    if (config->serialString)
    {
      struct UsbStringDescriptor * const stringDescriptor =
          malloc(sizeof(struct UsbStringDescriptor));

      stringDescriptor->data = config->serialString;
      stringDescriptor->descriptorType = DESCRIPTOR_STRING;
      stringDescriptor->length = 2 + strlen(config->serialString);
      ((struct UsbDescriptor *)root->children[2].children)[2].payload =
          (struct UsbDescriptorHeader *)stringDescriptor;
    }

    /* Product */
    if (config->productString)
    {
      struct UsbStringDescriptor * const stringDescriptor =
          malloc(sizeof(struct UsbStringDescriptor));

      stringDescriptor->data = config->productString;
      stringDescriptor->descriptorType = DESCRIPTOR_STRING;
      stringDescriptor->length = 2 + strlen(config->productString);
      ((struct UsbDescriptor *)root->children[2].children)[1].payload =
          (struct UsbDescriptorHeader *)stringDescriptor;
    }

    /* Manufacturer */
    if (config->vendorString)
    {
      struct UsbStringDescriptor * const stringDescriptor =
          malloc(sizeof(struct UsbStringDescriptor));

      stringDescriptor->data = config->vendorString;
      stringDescriptor->descriptorType = DESCRIPTOR_STRING;
      stringDescriptor->length = 2 + strlen(config->vendorString);
      ((struct UsbDescriptor *)root->children[2].children)[0].payload =
          (struct UsbDescriptorHeader *)stringDescriptor;
    }
  }

  return root;
}
/*----------------------------------------------------------------------------*/
static void freeDescriptors(const struct UsbDescriptor *root)
{
  /* Free string descriptors */
  bool stringFreed = false;

  for (uint8_t index = 0; index < 3; ++index)
  {
    if (root->children[2].children[index].payload !=
        rootDescriptor.children[2].children[index].payload)
    {
      free(root->children[2].children[index].payload);
      stringFreed = true;
    }
  }
  if (stringFreed)
    free(root->children[2].children);

  /* Free endpoint descriptors */
  free(root->children[1].children[8].payload);
  free(root->children[1].children[7].payload);
  free(root->children[1].children[5].payload);
  free(root->children[1].children);

  /* Free device descriptor */
  if (root->children[0].payload != rootDescriptor.children[0].payload)
    free(root->children[0].payload);

  /* Free root */
  if (root != &rootDescriptor)
    free(root);
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

  driver->descriptors = buildDescriptors(config);
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
  freeDescriptors(driver->descriptors);
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
static const struct UsbDescriptor *driverGetDescriptor(void *object)
{
  const struct CdcAcmBase * const driver = object;

  return driver->descriptors;
//  return &rootDescriptor;
}
/*----------------------------------------------------------------------------*/
static void driverSetSuspended(void *object, bool state)
{
  struct CdcAcmBase * const driver = object;
}
