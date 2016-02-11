/*
 * composite_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <usb/composite_device.h>
#include <usb/usb_control.h>
/*----------------------------------------------------------------------------*/
#define COMPOSITE_CONTROL_EP_SIZE 64
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static void driverUpdateStatus(void *, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CompositeDeviceProxy),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .updateStatus = driverUpdateStatus
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CompositeDeviceProxy = &driverTable;
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static void *devCreateEndpoint(void *, uint8_t);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum result devBind(void *, void *);
static void devUnbind(void *, const void *);
static uint8_t devGetConfiguration(const void *);
static void devSetConfiguration(void *, uint8_t);
static enum result devAppendDescriptor(void *, const void *);
static void devEraseDescriptor(void *, const void *);

static uint8_t compositeDevIndex(const void *);
/*----------------------------------------------------------------------------*/
static const struct CompositeDeviceClass deviceTable = {
    .base = {
        .size = sizeof(struct CompositeDevice),
        .init = devInit,
        .deinit = devDeinit,

        .createEndpoint = devCreateEndpoint,
        .setAddress = devSetAddress,
        .setConnected = devSetConnected,
        .bind = devBind,
        .unbind = devUnbind,
        .getConfiguration = devGetConfiguration,
        .setConfiguration = devSetConfiguration,

        .appendDescriptor = devAppendDescriptor,
        .eraseDescriptor = devEraseDescriptor
    },

    .index = compositeDevIndex
};
/*----------------------------------------------------------------------------*/
const struct CompositeDeviceClass * const CompositeDevice = &deviceTable;
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceDescriptor deviceDescriptor = {
    .length             = sizeof(struct UsbDeviceDescriptor),
    .descriptorType     = DESCRIPTOR_TYPE_DEVICE,
    .usb                = TO_LITTLE_ENDIAN_16(0x0200),
    .deviceClass        = USB_CLASS_MISCELLANEOUS,
    .deviceSubClass     = 0x02, /* Required for multiple IAD */
    .deviceProtocol     = 0x01, /* Required for multiple IAD */
    .maxPacketSize      = COMPOSITE_CONTROL_EP_SIZE,
    .idVendor           = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_VENDOR_ID),
    .idProduct          = TO_LITTLE_ENDIAN_16(CONFIG_USB_DEVICE_PRODUCT_ID),
    .device             = TO_LITTLE_ENDIAN_16(0x0100),
    .manufacturer       = 0,
    .product            = 0,
    .serialNumber       = 0,
    .numConfigurations  = 1
};
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CompositeDeviceConfig * const config = configBase;
  struct CompositeDeviceProxy * const driver = object;

  driver->owner = config->device;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object,
    const struct UsbSetupPacket *packet, const uint8_t *payload,
    uint16_t payloadLength, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct CompositeDeviceProxy * const driver = object;
  struct ListNode *currentNode = listFirst(&driver->owner->entries);
  struct UsbDriver *current;
  enum result res = E_INVALID;

  while (currentNode)
  {
    listData(&driver->owner->entries, currentNode, &current);
    res = usbDriverConfigure(current, packet, payload, payloadLength,
        response, responseLength, maxResponseLength);
    if (res == E_OK || (res != E_OK && res != E_INVALID))
      break;
    currentNode = listNext(currentNode);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void driverUpdateStatus(void *object, uint8_t status)
{
  struct CompositeDeviceProxy * const driver = object;
  struct ListNode *currentNode = listFirst(&driver->owner->entries);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&driver->owner->entries, currentNode, &current);
    usbDriverUpdateStatus(current, status);
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
static enum result devInit(void *object, const void *configBase)
{
  const struct CompositeDeviceConfig * const config = configBase;
  struct CompositeDevice * const device = object;
  const struct CompositeDeviceProxyConfig driverConfig = {
      .owner = device
  };
  enum result res;

  device->driver = init(CompositeDeviceProxy, &driverConfig);
  if (!device->driver)
    return E_ERROR;

  res = listInit(&device->entries, sizeof(const struct UsbDriver *));
  if (res != E_OK)
    return res;

  device->device = config->device;

  device->configDescriptor.length = sizeof(struct UsbConfigurationDescriptor);
  device->configDescriptor.descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;
  device->configDescriptor.totalLength = device->configDescriptor.length;
  device->configDescriptor.numInterfaces = 0;
  device->configDescriptor.configurationValue = 1;
  device->configDescriptor.configuration = 0;
  device->configDescriptor.attributes = CONFIGURATION_DESCRIPTOR_DEFAULT
      | CONFIGURATION_DESCRIPTOR_SELF_POWERED;
  device->configDescriptor.maxPower = (CONFIG_USB_DEVICE_CURRENT + 1) >> 1;

  res = usbDevAppendDescriptor(device->device, &deviceDescriptor);
  if (res != E_OK)
    return res;

  res = usbDevAppendDescriptor(device->device, &device->configDescriptor);
  if (res != E_OK)
    return res;

  if ((res = usbDevBind(device->device, device->driver)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct CompositeDevice * const device = object;

  usbDevEraseDescriptor(device->device, &device->configDescriptor);
  usbDevEraseDescriptor(device->device, &deviceDescriptor);

  assert(listEmpty(&device->entries));
  listDeinit(&device->entries);

  deinit(device->driver);
}
/*----------------------------------------------------------------------------*/
static void *devAllocate(void *object, uint8_t address)
{
  struct CompositeDevice * const device = object;

  return usbDevCreateEndpoint(device->device, address);
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  struct CompositeDevice * const device = object;

  usbDevSetAddress(device->device, address);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  struct CompositeDevice * const device = object;

  usbDevSetConnected(device->device, state);
}
/*----------------------------------------------------------------------------*/
static enum result devBind(void *object, void *driver)
{
  struct CompositeDevice * const device = object;

  return listPush(&device->entries, &driver);
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver)
{
  struct CompositeDevice * const device = object;
  struct ListNode * const node = listFind(&device->entries, &driver);

  if (node)
    listErase(&device->entries, node);
}
/*----------------------------------------------------------------------------*/
static uint8_t devGetConfiguration(const void *object)
{
  const struct CompositeDevice * const device = object;

  return usbDevGetConfiguration(device->device);
}
/*----------------------------------------------------------------------------*/
static void devSetConfiguration(void *object, uint8_t configuration)
{
  struct CompositeDevice * const device = object;

  usbDevSetConfiguration(device->device, configuration);
}
/*----------------------------------------------------------------------------*/
static enum result devAppendDescriptor(void *object, const void *descriptorBase)
{
  const struct UsbDescriptor * const descriptor = descriptorBase;
  struct CompositeDevice * const device = object;
  enum result res;

  if ((res = usbDevAppendDescriptor(device->device, descriptor)) != E_OK)
    return res;

  uint16_t total = fromLittleEndian16(device->configDescriptor.totalLength);

  switch (descriptor->descriptorType)
  {
    case DESCRIPTOR_TYPE_STRING:
      break;

    case DESCRIPTOR_TYPE_INTERFACE:
      ++device->configDescriptor.numInterfaces;
    default:
      total += descriptor->length;
      device->configDescriptor.totalLength = toLittleEndian16(total);
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devEraseDescriptor(void *object, const void *descriptorBase)
{
  const struct UsbDescriptor * const descriptor = descriptorBase;
  struct CompositeDevice * const device = object;

  usbDevEraseDescriptor(device->device, descriptor);

  uint16_t total = fromLittleEndian16(device->configDescriptor.totalLength);

  switch (descriptor->descriptorType)
  {
    case DESCRIPTOR_TYPE_STRING:
      break;

    case DESCRIPTOR_TYPE_INTERFACE:
      assert(device->configDescriptor.numInterfaces);

      --device->configDescriptor.numInterfaces;
    default:
      assert(descriptor->length <= total
          - sizeof(struct UsbConfigurationDescriptor));

      total -= descriptor->length;
      device->configDescriptor.totalLength = toLittleEndian16(total);
      break;
  }
}
/*----------------------------------------------------------------------------*/
static uint8_t compositeDevIndex(const void *object)
{
  const struct CompositeDevice * const device = object;

  return device->configDescriptor.numInterfaces;
}
