/*
 * requests.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <unicode.h>
#include <containers/list.h>
#include <usb/requests.h>
#include <usb/usb_device.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor *findEntry(const struct UsbDescriptor *,
    uint8_t, uint8_t *);
static enum result getDescriptorData(const struct UsbDescriptor *, uint16_t,
    uint16_t, uint8_t *, uint16_t *);
static struct UsbEndpoint *getEpByAddress(struct UsbDevice *, uint8_t);
static enum result handleStandardDeviceRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardEndpointRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardInterfaceRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result setDeviceConfig(struct UsbDevice *, uint8_t, uint8_t);
static enum result traverseConfigTree(struct UsbDevice *,
    const struct UsbDescriptor *, uint8_t, uint8_t, uint8_t *, uint8_t *);
/*----------------------------------------------------------------------------*/
static uint8_t bConfiguration = 0xFF; //FIXME
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor *findEntry(const struct UsbDescriptor *root,
    uint8_t type, uint8_t *offset)
{
  if (root->payload && root->payload->descriptorType == type)
  {
    if (*offset)
    {
      --(*offset);
    }
    else
    {
      return root;
    }
  }

  for (uint8_t index = 0; index < root->count; ++index)
  {
    const struct UsbDescriptor * const descriptor =
        findEntry(root->children + index, type, offset);

    if (descriptor)
      return descriptor;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
//FIXME wLangID
static enum result getDescriptorData(const struct UsbDescriptor *root,
    uint16_t keyword, uint16_t wLangID, uint8_t *buffer, uint16_t *length)
{
  uint8_t descriptorType, descriptorIndex;
  bool firstIndex;

  descriptorType = GET_DESC_TYPE(keyword);
  descriptorIndex = GET_DESC_INDEX(keyword);
  firstIndex = descriptorIndex == 0;

  const struct UsbDescriptor * const descriptor = findEntry(root,
      descriptorType, &descriptorIndex);

  if (!descriptor)
    return E_VALUE;

    uint8_t chunkLength;

  if (descriptor->payload->descriptorType == DESCRIPTOR_STRING && !firstIndex)
  {
    const struct UsbStringDescriptor * const payload =
        (const struct UsbStringDescriptor *)descriptor->payload;

    memcpy(buffer, descriptor->payload, 2);
    // FIXME Ends with null character, rewrite to use returned length
    uToUtf16((char16_t *)(buffer + 2), (const char *)payload->data,
        (descriptor->payload->length - 2) + 1);
    *length = 2 + (descriptor->payload->length - 2) * 2;
    buffer[0] = *length;
  }
  else
  {
    //TODO Rewrite
    if (descriptor->payload->descriptorType == DESCRIPTOR_CONFIGURATION)
    {
      const struct UsbConfigurationDescriptor * const payload =
          (const struct UsbConfigurationDescriptor *)descriptor->payload;

      chunkLength = payload->totalLength;
    }
    else
    {
      chunkLength = descriptor->payload->length;
    }

    *length = chunkLength;

    memcpy(buffer, descriptor->payload, descriptor->payload->length);
    buffer += descriptor->payload->length;
    chunkLength -= descriptor->payload->length;

    for (uint8_t index = 0; chunkLength && index < descriptor->count; ++index)
    {
      const struct UsbDescriptor * const child = descriptor->children + index;

      assert(child->payload);
      assert(chunkLength >= child->payload->length);

      memcpy(buffer, child->payload, child->payload->length);
      buffer += child->payload->length;
      chunkLength -= child->payload->length;
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static struct UsbEndpoint *getEpByAddress(struct UsbDevice *device,
    uint8_t address)
{
  const struct ListNode *current = listFirst(&device->device->endpoints);
  struct UsbEndpoint *endpoint = 0;

  while (current)
  {
    listData(&device->device->endpoints, current, &endpoint);

    if (endpoint->address == address)
      break;
    current = listNext(current);
  }

  return endpoint;
}
/*----------------------------------------------------------------------------*/
static enum result handleStandardDeviceRequest(struct UsbDevice *device,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Bit 0: self-powered */
      /* bit 1: remote wakeup */
      buffer[0] = 0; //FIXME
      buffer[1] = 0;
      *length = 2;
      break;

    case REQUEST_SET_ADDRESS:
      usbDevSetAddress(device, packet->value);
      usbTrace("requests: set address %d", packet->value);
      *length = 0;
      break;

    case REQUEST_GET_DESCRIPTOR:
    {
      const struct UsbDescriptor * const root =
          usbDriverGetDescriptor(device->driver);

      usbTrace("requests: get descriptor %d:%d, length %u",
          GET_DESC_TYPE(packet->value), GET_DESC_INDEX(packet->value),
          packet->length);

      return getDescriptorData(root, packet->value, packet->index, buffer,
          length);
    }

    case REQUEST_GET_CONFIGURATION:
      buffer[0] = bConfiguration;
      *length = 1;
      break;

    case REQUEST_SET_CONFIGURATION:
    {
      const uint8_t configuration = packet->value & 0xFF;
      const enum result res = setDeviceConfig(device, configuration, 0);

      if (res != E_OK)
      {
        usbTrace("requests: configuration %d setup failed", configuration);
        return res;
      }

      usbTrace("requests: configuration %d set successfully", configuration);
      bConfiguration = configuration;
      *length = 0;
      break;
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_REMOTE_WAKEUP)
      {
        // put DEVICE_REMOTE_WAKEUP code here
      }
      if (packet->value == FEATURE_TEST_MODE)
      {
        // put TEST_MODE code here
      }
      return E_ERROR;

    case REQUEST_SET_DESCRIPTOR:
//    printf("Device req %d not implemented\n", pSetup->request);
      return E_ERROR;

    default:
//    printf("Illegal device req %d\n", pSetup->request);
      return E_ERROR;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result handleStandardEndpointRequest(struct UsbDevice *device,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  struct UsbEndpoint *endpoint = getEpByAddress(device, packet->index);

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Endpoint is halted or not */
      buffer[0] = usbEpIsStalled(endpoint) ? 1 : 0;
      buffer[1] = 0;
      *length = 2;
      return E_OK;

    case REQUEST_CLEAR_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Clear halt by unstalling */
        usbEpSetStalled(endpoint, false);
        *length = 0;
        usbTrace("requests: unstall endpoint %02X", endpoint->address);
        return E_OK;
      }
      break;

    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Set halt by stalling */
        usbEpSetStalled(endpoint, true);
        *length = 0;
        usbTrace("requests: stall endpoint %02X", endpoint->address);
        return E_OK;
      }
      break;

    default:
      usbTrace("requests: unsupported request %02X to endpoint %02X",
          packet->request, endpoint->address);
      break;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result handleStandardInterfaceRequest(struct UsbDevice *device,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      buffer[0] = 0;
      buffer[1] = 0;
      *length = 2;
      break;

    case REQUEST_GET_INTERFACE: // TODO use bNumInterfaces
      buffer[0] = 0;
      *length = 1;
      break;

    case REQUEST_SET_INTERFACE: // TODO use bNumInterfaces
      usbTrace("requests: set interface %d", packet->value);
      if (packet->value != 0)
        return E_ERROR; //FIXME
      *length = 0;
      break;

    default:
      usbTrace("requests: unsupported interface request %02X", packet->request);
      return E_ERROR;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result setDeviceConfig(struct UsbDevice *device,
    uint8_t configuration, uint8_t alternativeSettings)
{
  if (configuration == 0)
  {
    usbDevSetConfigured(device, false);
  }
  else
  {
    if (!device->driver)
      return E_ERROR; //TODO Assert?

    const struct UsbDescriptor * const root =
        usbDriverGetDescriptor(device->driver);
    enum result res;

    uint8_t currentConfig = 0xFF;
    uint8_t currentSettings = 0xFF;

    res = traverseConfigTree(device, root, configuration, alternativeSettings,
        &currentConfig, &currentSettings);
    if (res != E_OK)
      return res;

    bConfiguration = configuration;
    usbDevSetConfigured(device, true);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result traverseConfigTree(struct UsbDevice *device,
    const struct UsbDescriptor *root, uint8_t configuration, uint8_t settings,
    uint8_t *currentConfiguration, uint8_t *currentSettings)
{
  if (root->payload)
  {
    if (root->payload->descriptorType == DESCRIPTOR_CONFIGURATION)
    {
      const struct UsbConfigurationDescriptor * const descriptor =
          (const struct UsbConfigurationDescriptor *)root->payload;

      *currentConfiguration = descriptor->configurationValue;
    }
    else if (root->payload->descriptorType == DESCRIPTOR_INTERFACE)
    {
      const struct UsbInterfaceDescriptor * const descriptor =
          (const struct UsbInterfaceDescriptor *)root->payload;

      *currentSettings = descriptor->alternateSettings;
    }
    else if (root->payload->descriptorType == DESCRIPTOR_ENDPOINT
        && *currentConfiguration == configuration
        && *currentSettings == settings)
    {
      const struct UsbEndpointDescriptor * const descriptor =
          (const struct UsbEndpointDescriptor *)root->payload;

      //FIXME Byte order
      //TODO Set endpoint size descriptor->maxPacketSize
      struct UsbEndpoint * const endpoint = getEpByAddress(device,
          descriptor->endpointAddress);

      assert(endpoint);
      usbEpSetEnabled(endpoint, true);
    }
  }

  for (uint8_t index = 0; index < root->count; ++index)
  {
    const enum result res = traverseConfigTree(device, root->children + index,
        configuration, settings, currentConfiguration, currentSettings);

    if (res != E_OK)
      return res;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result usbHandleStandardRequest(struct UsbDevice *device,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  switch (REQUEST_RECIPIENT_VALUE(packet->requestType))
  {
    case REQUEST_RECIPIENT_DEVICE:
      return handleStandardDeviceRequest(device, packet, buffer, length);

    case REQUEST_RECIPIENT_INTERFACE:
      return handleStandardInterfaceRequest(device, packet, buffer, length);

    case REQUEST_RECIPIENT_ENDPOINT:
      return handleStandardEndpointRequest(device, packet, buffer, length);

    default:
      return E_INVALID;
  }
}
