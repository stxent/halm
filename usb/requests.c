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
static const struct UsbDescriptor **findEntry(const struct UsbDescriptor **,
    uint8_t, uint8_t);
static enum result getDescriptorData(const struct UsbDescriptor **,
    uint16_t, uint16_t, uint8_t *, uint16_t *);
static struct UsbEndpoint *getEpByAddress(struct UsbDevice *, uint8_t);
static enum result handleStandardDeviceRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardEndpointRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardInterfaceRequest(struct UsbDevice *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result setDeviceConfig(struct UsbDevice *, uint8_t, uint8_t);
static enum result traverseConfigArray(struct UsbDevice *,
    const struct UsbDescriptor **, uint8_t, uint8_t);
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor **findEntry(const struct UsbDescriptor **root,
    uint8_t type, uint8_t offset)
{
  while (*root)
  {
    if ((*root)->descriptorType == type)
    {
      if (!offset)
        return root;
      else
        --offset;
    }
    ++root;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
//FIXME wLangID
static enum result getDescriptorData(const struct UsbDescriptor **root,
    uint16_t keyword, uint16_t wLangID, uint8_t *buffer, uint16_t *length)
{
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const struct UsbDescriptor * const *entry = findEntry(root,
      descriptorType, descriptorIndex);

  if (!entry)
    return E_VALUE;

  if ((*entry)->descriptorType == DESCRIPTOR_STRING && descriptorIndex)
  {
    const struct UsbStringDescriptor * const data =
        (const struct UsbStringDescriptor *)(*entry);
    const unsigned int stringLength = uLengthToUtf16(data->data) + 1;

    uToUtf16((char16_t *)(buffer + 2), data->data, stringLength);
    buffer[0] = 2 + (stringLength << 1);
    buffer[1] = DESCRIPTOR_STRING;
    *length = buffer[0];
    return E_OK;
  }

  uint16_t chunkLength = 0;

  if ((*entry)->descriptorType == DESCRIPTOR_CONFIGURATION)
  {
    const struct UsbConfigurationDescriptor * const data =
        (const struct UsbConfigurationDescriptor *)(*entry);

    chunkLength = data->totalLength;
  }
  else
  {
    chunkLength = (*entry)->length;
  }

  if (chunkLength)
    *length = chunkLength;

  while (*entry && chunkLength)
  {
    const uint8_t entryLength = (*entry)->length;

    memcpy(buffer, *entry, entryLength);
    buffer += entryLength;
    chunkLength -= entryLength;

    ++entry;
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
      const struct UsbDescriptor ** const root =
          usbDriverGetDescriptor(device->driver);

      usbTrace("requests: get descriptor %d:%d, length %u",
          DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
          packet->length);

      return getDescriptorData(root, packet->value, packet->index, buffer,
          length);
    }

    case REQUEST_GET_CONFIGURATION:
      buffer[0] = device->currentConfiguration;
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
      else
      {
        usbTrace("requests: configuration %d set successfully", configuration);
        *length = 0;
        break;
      }
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

    default:
      usbTrace("requests: unsupported device request %02X", packet->request);
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

    const struct UsbDescriptor ** const root =
        usbDriverGetDescriptor(device->driver);
    const enum result res = traverseConfigArray(device, root,
        configuration, alternativeSettings);

    if (res != E_OK)
      return res;

    device->currentConfiguration = configuration;
    usbDevSetConfigured(device, true);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result traverseConfigArray(struct UsbDevice *device,
    const struct UsbDescriptor **root, uint8_t configuration, uint8_t settings)
{
  //FIXME Choose default values
  uint8_t currentConfiguration = 0xFF;
  uint8_t currentSettings = 0xFF;

  while (*root)
  {
    if ((*root)->descriptorType == DESCRIPTOR_CONFIGURATION)
    {
      const struct UsbConfigurationDescriptor * const data =
          (const struct UsbConfigurationDescriptor *)(*root);

      currentConfiguration = data->configurationValue;
    }
    else if ((*root)->descriptorType == DESCRIPTOR_INTERFACE)
    {
      const struct UsbInterfaceDescriptor * const data =
          (const struct UsbInterfaceDescriptor *)(*root);

      currentSettings = data->alternateSettings;
    }
    else if ((*root)->descriptorType == DESCRIPTOR_ENDPOINT
        && currentConfiguration == configuration
        && currentSettings == settings)
    {
      const struct UsbEndpointDescriptor * const data =
          (const struct UsbEndpointDescriptor *)(*root);

      //FIXME Byte order
      //TODO Set endpoint size descriptor->maxPacketSize
      struct UsbEndpoint * const endpoint = getEpByAddress(device,
          data->endpointAddress);

      assert(endpoint);
      usbEpSetEnabled(endpoint, true);
    }

    ++root;
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
