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
    .size = sizeof(struct CompositeDevice),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .updateStatus = driverUpdateStatus
};
/*----------------------------------------------------------------------------*/
const struct UsbDriverClass * const CompositeDevice = &driverTable;
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
void compositeDeviceUpdate(struct CompositeDevice *driver,
    uint8_t interfaceCount, uint16_t totalLength)
{
  driver->configDescriptor.numInterfaces = interfaceCount;
  driver->configDescriptor.totalLength = toLittleEndian16(totalLength);
}
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *object, const void *configBase)
{
  const struct CompositeDeviceConfig * const config = configBase;
  struct CompositeDevice * const driver = object;
  enum result res;

  driver->control = config->control;

  driver->configDescriptor.length = sizeof(struct UsbConfigurationDescriptor);
  driver->configDescriptor.descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;
  driver->configDescriptor.totalLength = 0;
  driver->configDescriptor.numInterfaces = 0;
  driver->configDescriptor.configurationValue = 1;
  driver->configDescriptor.configuration = 0;
  driver->configDescriptor.attributes = CONFIGURATION_DESCRIPTOR_DEFAULT
      | CONFIGURATION_DESCRIPTOR_SELF_POWERED;
  driver->configDescriptor.maxPower = (CONFIG_USB_DEVICE_CURRENT + 1) >> 1;

  res = usbControlAppendDescriptor(driver->control, &deviceDescriptor);
  if (res != E_OK)
    return res;

  res = usbControlAppendDescriptor(driver->control, &driver->configDescriptor);
  if (res != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void driverDeinit(void *object)
{
  struct CompositeDevice * const driver = object;

  usbControlEraseDescriptor(driver->control, &driver->configDescriptor);
  usbControlEraseDescriptor(driver->control, &deviceDescriptor);
}
/*----------------------------------------------------------------------------*/
static enum result driverConfigure(void *object __attribute__((unused)),
    const struct UsbSetupPacket *packet __attribute__((unused)),
    const uint8_t *payload __attribute__((unused)),
    uint16_t payloadLength __attribute__((unused)),
    uint8_t *response __attribute__((unused)),
    uint16_t *responseLength __attribute__((unused)),
    uint16_t maxResponseLength __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static void driverUpdateStatus(void *object __attribute__((unused)),
    uint8_t status __attribute__((unused)))
{

}
