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
static enum Result handleDeviceRequest(struct CompositeDeviceProxy *,
    const struct UsbSetupPacket *, void *, uint16_t *, uint16_t);
static enum Result lookupDescriptor(struct CompositeDeviceProxy *,
    uint8_t, void *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *, const void *);
static void driverDeinit(void *);
static enum Result driverControl(void *, const struct UsbSetupPacket *,
    void *, uint16_t *, uint16_t);
static const UsbDescriptorFunctor *driverDescribe(const void *);
static void driverNotify(void *, unsigned int);
/*----------------------------------------------------------------------------*/
static const struct UsbDriverClass * const CompositeDeviceProxy =
    &(const struct UsbDriverClass){
    .size = sizeof(struct CompositeDeviceProxy),
    .init = driverInit,
    .deinit = driverDeinit,

    .control = driverControl,
    .describe = driverDescribe,
    .notify = driverNotify
};
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *, const void *);
static void devDeinit(void *);
static void *devCreateEndpoint(void *, uint8_t);
static uint8_t devGetInterface(const void *);
static void devSetAddress(void *, uint8_t);
static void devSetConnected(void *, bool);
static enum Result devBind(void *, void *);
static void devUnbind(void *, const void *);
static void devSetPower(void *, uint16_t);
static enum UsbSpeed devGetSpeed(const void *);
static UsbStringNumber devStringAppend(void *, struct UsbString);
static void devStringErase(void *, struct UsbString);
/*----------------------------------------------------------------------------*/
const struct UsbDeviceClass * const CompositeDevice =
    &(const struct UsbDeviceClass){
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
    .getSpeed = devGetSpeed,

    .stringAppend = devStringAppend,
    .stringErase = devStringErase
};
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

  PointerListNode *current = pointerListFront(&driver->owner->entries);

  while (current)
  {
    struct UsbDriver * const entry = *pointerListData(current);

    payloadPosition += extendConfigurationDescriptor(entry, payloadPosition);
    current = pointerListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static void computeDescriptionProperties(const void *driver, uint16_t *length,
    uint8_t *interfaces)
{
  const UsbDescriptorFunctor *entry = usbDriverDescribe(driver);
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
        /* Falls through */
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
  const UsbDescriptorFunctor *entry = usbDriverDescribe(driver);
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
static enum Result handleDeviceRequest(struct CompositeDeviceProxy *driver,
    const struct UsbSetupPacket *packet, void *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  enum Result res = E_INVALID;

  if (packet->request == REQUEST_GET_DESCRIPTOR)
  {
    usbTrace("composite: get descriptor %u:%u, length %u",
        DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
        packet->length);

    const uint8_t descriptorType = DESCRIPTOR_TYPE(packet->value);

    if (descriptorType == DESCRIPTOR_TYPE_DEVICE)
    {
      const size_t descriptorSize = sizeof(struct UsbDeviceDescriptor);
      assert(descriptorSize <= maxResponseLength);

      memset(response, 0, descriptorSize);
      deviceDescriptor(driver, response, response);
      *responseLength = descriptorSize;

      res = E_OK;
    }
    else if (descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
    {
      const size_t descriptorSize = driver->owner->configurationLength;
      assert(descriptorSize <= maxResponseLength);

      memset(response, 0, descriptorSize);
      configDescriptor(driver, response, response);
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
static enum Result lookupDescriptor(struct CompositeDeviceProxy *driver,
    uint8_t type, void *response, uint16_t *responseLength,
    uint16_t maxResponseLength)
{
  PointerListNode *current = pointerListFront(&driver->owner->entries);

  while (current)
  {
    struct UsbDriver * const entry = *pointerListData(current);

    const enum Result res = usbExtractDescriptorData(entry, type,
        response, responseLength, maxResponseLength);

    if (res == E_OK)
      return E_OK;

    current = pointerListNext(current);
  }

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result driverInit(void *object, const void *configBase)
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
static enum Result driverControl(void *object,
    const struct UsbSetupPacket *packet, void *buffer,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  struct CompositeDeviceProxy * const driver = object;
  const uint8_t recipient = REQUEST_RECIPIENT_VALUE(packet->requestType);
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);
  enum Result res = E_INVALID;

  if (type == REQUEST_TYPE_STANDARD && recipient == REQUEST_RECIPIENT_DEVICE)
  {
    res = handleDeviceRequest(driver, packet, buffer, responseLength,
        maxResponseLength);
  }

  if (res == E_INVALID)
  {
    PointerListNode *current = pointerListFront(&driver->owner->entries);

    while (current)
    {
      struct UsbDriver * const entry = *pointerListData(current);

      res = usbDriverControl(entry, packet, buffer, responseLength,
          maxResponseLength);
      if (res == E_OK || res != E_INVALID)
        break;
      current = pointerListNext(current);
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void driverNotify(void *object, unsigned int event)
{
  struct CompositeDeviceProxy * const driver = object;
  PointerListNode *current = pointerListFront(&driver->owner->entries);

  while (current)
  {
    struct UsbDriver * const entry = *pointerListData(current);

    usbDriverNotify(entry, event);
    current = pointerListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *driverDescribe(const void *object
    __attribute__((unused)))
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static enum Result devInit(void *object, const void *configBase)
{
  const struct CompositeDeviceConfig * const config = configBase;
  struct CompositeDevice * const device = object;
  const struct CompositeDeviceProxyConfig driverConfig = {
      .owner = device
  };

  device->driver = init(CompositeDeviceProxy, &driverConfig);
  if (!device->driver)
    return E_ERROR;

  pointerListInit(&device->entries);

  device->parent = config->device;
  device->configurationLength = sizeof(struct UsbConfigurationDescriptor);
  device->interfaceCount = 0;

  return usbDevBind(device->parent, device->driver);
}
/*----------------------------------------------------------------------------*/
static void devDeinit(void *object)
{
  struct CompositeDevice * const device = object;

  assert(pointerListEmpty(&device->entries));
  pointerListDeinit(&device->entries);

  deinit(device->driver);
}
/*----------------------------------------------------------------------------*/
static void *devCreateEndpoint(void *object, uint8_t address)
{
  struct CompositeDevice * const device = object;
  return usbDevCreateEndpoint(device->parent, address);
}
/*----------------------------------------------------------------------------*/
static uint8_t devGetInterface(const void *object)
{
  return ((struct CompositeDevice *)object)->interfaceCount;
}
/*----------------------------------------------------------------------------*/
static void devSetAddress(void *object, uint8_t address)
{
  usbDevSetAddress(((struct CompositeDevice *)object)->parent, address);
}
/*----------------------------------------------------------------------------*/
static void devSetConnected(void *object, bool state)
{
  usbDevSetConnected(((struct CompositeDevice *)object)->parent, state);
}
/*----------------------------------------------------------------------------*/
static enum Result devBind(void *object, void *driver)
{
  struct CompositeDevice * const device = object;
  uint16_t length;
  uint8_t interfaces;

  computeDescriptionProperties(driver, &length, &interfaces);

  assert(device->interfaceCount + interfaces <= UCHAR_MAX);
  assert(device->configurationLength + length <= USHRT_MAX);

  if (pointerListPushBack(&device->entries, driver))
  {
    device->interfaceCount += interfaces;
    device->configurationLength += length;
    return E_OK;
  }
  else
    return E_MEMORY;
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
  assert(pointerListFind(&device->entries, (void *)driver));

  pointerListErase(&device->entries, (void *)driver);
  device->interfaceCount -= interfaces;
  device->configurationLength -= length;
}
/*----------------------------------------------------------------------------*/
static void devSetPower(void *object, uint16_t current)
{
  usbDevSetPower(((const struct CompositeDevice *)object)->parent, current);
}
/*----------------------------------------------------------------------------*/
static enum UsbSpeed devGetSpeed(const void *object)
{
  return usbDevGetSpeed(((const struct CompositeDevice *)object)->parent);
}
/*----------------------------------------------------------------------------*/
static UsbStringNumber devStringAppend(void *object, struct UsbString string)
{
  return usbDevStringAppend(((struct CompositeDevice *)object)->parent, string);
}
/*----------------------------------------------------------------------------*/
static void devStringErase(void *object, struct UsbString string)
{
  usbDevStringErase(((struct CompositeDevice *)object)->parent, string);
}
