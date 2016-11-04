/*
 * composite_device.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <limits.h>
#include <xcore/memory.h>
#include <halm/usb/composite_device.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define COMPOSITE_CONTROL_EP_SIZE 64
/*----------------------------------------------------------------------------*/
struct CompositeDeviceProxyConfig
{
  /** Mandatory: Composite Device object. */
  struct CompositeDevice *owner;
};
/*----------------------------------------------------------------------------*/
struct CompositeDeviceProxy
{
  struct UsbDriver base;

  struct CompositeDevice *owner;
};
/*----------------------------------------------------------------------------*/
static void deviceDescriptor(const void *, struct UsbDescriptor *, void *);
static void configDescriptor(const void *, struct UsbDescriptor *, void *);
/*----------------------------------------------------------------------------*/
static void computeDescriptionProperties(const void *, uint16_t *, uint8_t *);
static uint16_t extendConfigurationDescriptor(const void *, uint8_t *);
static enum result handleDeviceRequest(struct CompositeDeviceProxy *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum result lookupDescriptor(struct CompositeDeviceProxy *,
    uint8_t, uint8_t *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum result driverConfigure(void *, const struct UsbSetupPacket *,
    const uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
static const usbDescriptorFunctor *driverDescribe(const void *);
static void driverEvent(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass driverTable = {
    .size = sizeof(struct CompositeDeviceProxy),
    .init = driverInit,
    .deinit = driverDeinit,

    .configure = driverConfigure,
    .describe = driverDescribe,
    .event = driverEvent
};
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass * const CompositeDeviceProxy = &driverTable;
/*----------------------------------------------------------------------------*/
static enum result devInit(void *, const void *);
static void devDeinit(void *);
static void *devCreateEndpoint(void *, uint8_t);
static uint8_t devGetInterface(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum result devBind(void *, void *);
static void devUnbind(void *, const void *);
static void devSetPower(void *, uint16_t);
static enum usbSpeed devGetSpeed(const void *);
/*----------------------------------------------------------------------------*/
static const struct UsbDeviceClass deviceTable = {
    .size = sizeof(struct CompositeDevice),
    .init = devInit,
    .deinit = devDeinit,

    .createEndpoint = devCreateEndpoint,
    .getInterface = devGetInterface,
    .setAddress = devSetAddress,
    .setConnected = devSetConnected,

    .bind = devBind,
    .unbind = devUnbind,

    .setPower = devSetPower,
    .getSpeed = devGetSpeed
};
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const CompositeDevice = &deviceTable;
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
    descriptor->deviceClass = USB_CLASS_MISCELLANEOUS;
    descriptor->deviceSubClass = 0x02; /* Required for multiple IAD */
    descriptor->deviceProtocol = 0x01; /* Required for multiple IAD */
    descriptor->maxPacketSize = TO_LITTLE_ENDIAN_16(COMPOSITE_CONTROL_EP_SIZE);
    descriptor->device = TO_LITTLE_ENDIAN_16(0x0100);
    descriptor->numConfigurations = 1;
  }
}
/*----------------------------------------------------------------------------*/
static void configDescriptor(const void *object, struct UsbDescriptor *header,
    void *payload)
{
  const struct CompositeDeviceProxy * const driver = object;
  const struct CompositeDevice * const device = driver->owner;

  header->length = sizeof(struct UsbConfigurationDescriptor);
  header->descriptorType = DESCRIPTOR_TYPE_CONFIGURATION;

  if (!payload)
    return;

  struct UsbConfigurationDescriptor * const descriptor = payload;

  descriptor->totalLength = toLittleEndian16(device->configurationLength);
  descriptor->numInterfaces = device->interfaceCount;
  descriptor->configurationValue = 1;

  uint8_t *payloadPosition = (uint8_t *)payload
      + sizeof(struct UsbConfigurationDescriptor);

  struct ListNode *currentNode = listFirst(&driver->owner->entries);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&driver->owner->entries, currentNode, &current);
    payloadPosition += extendConfigurationDescriptor(current, payloadPosition);
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
static void computeDescriptionProperties(const void *driver, uint16_t *length,
    uint8_t *interfaces)
{
  const usbDescriptorFunctor *entry = usbDriverDescribe(driver);
  uint16_t descriptionLength = 0;
  uint8_t descriptionInterfaces = 0;

  while (*entry)
  {
    struct UsbDescriptor header;

    (*entry)(driver, &header, 0);

    switch (header.descriptorType)
    {
      case DESCRIPTOR_TYPE_DEVICE:
      case DESCRIPTOR_TYPE_CONFIGURATION:
      case DESCRIPTOR_TYPE_STRING:
        break;

      case DESCRIPTOR_TYPE_INTERFACE:
        ++descriptionInterfaces;
        /* No break */
      default:
        descriptionLength += header.length;
        break;
    }
    ++entry;
  }

  *length = descriptionLength;
  *interfaces = descriptionInterfaces;
}
/*----------------------------------------------------------------------------*/
static uint16_t extendConfigurationDescriptor(const void *driver,
    uint8_t *response)
{
  const usbDescriptorFunctor *entry = usbDriverDescribe(driver);
  uint16_t total = 0;

  while (*entry)
  {
    struct UsbDescriptor header;

    (*entry)(driver, &header, 0);

    switch (header.descriptorType)
    {
      case DESCRIPTOR_TYPE_DEVICE:
      case DESCRIPTOR_TYPE_CONFIGURATION:
      case DESCRIPTOR_TYPE_STRING:
        break;

      default:
        (*entry)(driver, (struct UsbDescriptor *)response, response);
        total += header.length;
        response += header.length;
        break;
    }

    ++entry;
  }

  return total;
}
/*----------------------------------------------------------------------------*/
static enum result handleDeviceRequest(struct CompositeDeviceProxy *driver,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum result res = E_INVALID;

  if (packet->request == REQUEST_GET_DESCRIPTOR)
  {
    const uint8_t descriptorType = DESCRIPTOR_TYPE(packet->value);

    usbTrace("composite: get descriptor %u:%u, length %u",
        descriptorType, descriptorIndex, packet->length);

    if (descriptorType == DESCRIPTOR_TYPE_DEVICE)
    {
      const size_t descriptorSize = sizeof(struct UsbDeviceDescriptor);
      assert(descriptorSize <= maxResponseLength);

      memset(response, 0, descriptorSize);
      deviceDescriptor(driver, (struct UsbDescriptor *)response, response);
      *responseLength = descriptorSize;

      res = E_OK;
    }
    else if (descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
    {
      const size_t descriptorSize = driver->owner->configurationLength;
      assert(descriptorSize <= maxResponseLength);

      memset(response, 0, descriptorSize);
      configDescriptor(driver, (struct UsbDescriptor *)response, response);
      *responseLength = descriptorSize;

      res = E_OK;
    }
    else
    {
      res = lookupDescriptor(driver, descriptorType, response, responseLength,
          maxResponseLength);
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum result lookupDescriptor(struct CompositeDeviceProxy *driver,
    uint8_t type, uint8_t *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  struct ListNode *currentNode = listFirst(&driver->owner->entries);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&driver->owner->entries, currentNode, &current);

    const enum result res = usbExtractDescriptorData(current, type,
        response, responseLength, maxResponseLength);

    if (res == E_OK)
      return E_OK;

    currentNode = listNext(currentNode);
  }

  return E_VALUE;
}
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
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);
  enum result res = E_INVALID;

  if (type == REQUEST_TYPE_STANDARD && recipient == REQUEST_RECIPIENT_DEVICE)
  {
    res = handleDeviceRequest(driver, packet, response, responseLength,
        maxResponseLength);
  }

  if (res == E_INVALID)
  {
    struct ListNode *currentNode = listFirst(&driver->owner->entries);
    struct UsbDriver *current;

    while (currentNode)
    {
      listData(&driver->owner->entries, currentNode, &current);
      res = usbDriverConfigure(current, packet, payload, payloadLength,
          response, responseLength, maxResponseLength);
      if (res == E_OK || (res != E_OK && res != E_INVALID))
        break;
      currentNode = listNext(currentNode);
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void driverEvent(void *object, unsigned int event)
{
  struct CompositeDeviceProxy * const driver = object;
  struct ListNode *currentNode = listFirst(&driver->owner->entries);
  struct UsbDriver *current;

  while (currentNode)
  {
    listData(&driver->owner->entries, currentNode, &current);
    usbDriverEvent(current, event);
    currentNode = listNext(currentNode);
  }
}
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
{
  return 0;
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
  device->configurationLength = sizeof(struct UsbConfigurationDescriptor);
  device->interfaceCount = 0;

  if ((res = usbDevBind(device->device, device->driver)) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct CompositeDevice * const device = object;

  assert(listEmpty(&device->entries));
  listDeinit(&device->entries);

  deinit(device->driver);
}
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct CompositeDevice * const device = object;

  return usbDevCreateEndpoint(device->device, address);
}
/*----------------------------------------------------------------------------*/
static uint8_t devGetInterface(const void *object)
{
  const struct CompositeDevice * const device = object;

  return device->interfaceCount;
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
  uint16_t length;
  uint8_t interfaces;

  computeDescriptionProperties(driver, &length, &interfaces);

  assert(device->interfaceCount + interfaces <= UCHAR_MAX);
  assert(device->configurationLength + length <= USHRT_MAX);

  const enum result res = listPush(&device->entries, &driver);

  if (res == E_OK)
  {
    device->interfaceCount += interfaces;
    device->configurationLength += length;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void devUnbind(void *object, const void *driver)
{
  struct CompositeDevice * const device = object;
  uint16_t length;
  uint8_t interfaces;

  computeDescriptionProperties(driver, &length, &interfaces);

  assert(device->interfaceCount >= interfaces);
  assert(device->configurationLength >= length);

  struct ListNode * const node = listFind(&device->entries, &driver);

  if (node)
  {
    listErase(&device->entries, node);
    device->interfaceCount -= interfaces;
    device->configurationLength -= length;
  }
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  usbDevSetPower(((const struct CompositeDevice *)object)->device, current);
}
/*----------------------------------------------------------------------------*/
static enum usbSpeed devGetSpeed(const void *object)
{
  return usbDevGetSpeed(((const struct CompositeDevice *)object)->device);
}
