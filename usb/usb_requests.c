/*
 * usb_requests.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unicode.h>
#include <usb/usb_control.h>
#include <usb/usb_requests.h>
#include <usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static const struct UsbDescriptor **findEntry(const struct UsbDescriptor **,
    uint8_t, uint8_t);
static enum result getDescriptorData(const struct UsbDescriptor **,
    uint16_t, uint16_t, uint8_t *, uint16_t *);
static enum result handleStandardDeviceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardEndpointRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result handleStandardInterfaceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *);
static enum result setDeviceConfig(struct UsbControl *, uint8_t, uint8_t);
static enum result traverseConfigArray(struct UsbControl *,
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
/* TODO Add support for language identifier */
static enum result getDescriptorData(const struct UsbDescriptor **root,
    uint16_t keyword, uint16_t language, uint8_t *buffer, uint16_t *length)
{
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const struct UsbDescriptor * const *entry = findEntry(root,
      descriptorType, descriptorIndex);

  if (!entry)
    return E_VALUE;

  if ((*entry)->descriptorType == DESCRIPTOR_TYPE_STRING && descriptorIndex)
  {
    const char * const data = (const char *)((*entry)->data);
    const unsigned int stringLength = uLengthToUtf16(data);

    /* Check descriptor length */
    if (2 + (stringLength << 1) > *length)
      return E_VALUE;

    uToUtf16((char16_t *)(buffer + 2), data, stringLength + 1);
    buffer[0] = 2 + (stringLength << 1);
    buffer[1] = DESCRIPTOR_TYPE_STRING;
    *length = buffer[0];
    return E_OK;
  }

  uint16_t chunkLength = 0;

  if ((*entry)->descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
  {
    const struct UsbConfigurationDescriptor * const data =
        (const struct UsbConfigurationDescriptor *)(*entry);

    chunkLength = data->totalLength;
  }
  else
  {
    chunkLength = (*entry)->length;
  }

  if (chunkLength > *length)
    return E_VALUE;

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
static enum result handleStandardDeviceRequest(struct UsbControl *control,
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
      usbDevSetAddress(control->base, packet->value);
      usbTrace("requests: set address %d", packet->value);
      *length = 0;
      break;

    case REQUEST_GET_DESCRIPTOR:
    {
      const struct UsbDescriptor ** const root =
          usbDriverGetDescriptors(control->driver);

      usbTrace("requests: get descriptor %d:%d, length %u",
          DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
          packet->length);

      return getDescriptorData(root, packet->value, packet->index, buffer,
          length);
    }

    case REQUEST_GET_CONFIGURATION:
      buffer[0] = control->currentConfiguration;
      *length = 1;
      break;

    case REQUEST_SET_CONFIGURATION:
    {
      const uint8_t configuration = packet->value & 0xFF;
      const enum result res = setDeviceConfig(control, configuration, 0);

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
static enum result handleStandardEndpointRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  struct UsbEndpoint *endpoint = usbDevAllocate(control->base, packet->index);

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Endpoint is halted or not */
      buffer[0] = (uint8_t)usbEpIsStalled(endpoint);
      buffer[1] = 0; /* Must be set to zero */
      *length = 2;
      return E_OK;

    case REQUEST_CLEAR_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Clear halt by unstalling */
        usbEpSetStalled(endpoint, false);
        *length = 0;
        usbTrace("requests: unstall endpoint %02X", packet->index);
        return E_OK;
      }
      break;

    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Set halt by stalling */
        usbEpSetStalled(endpoint, true);
        *length = 0;
        usbTrace("requests: stall endpoint %02X", packet->index);
        return E_OK;
      }
      break;

    default:
      usbTrace("requests: unsupported request %02X to endpoint %02X",
          packet->request, packet->index);
      break;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result handleStandardInterfaceRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Two bytes are reserved for future use and must be set to zero */
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
static enum result setDeviceConfig(struct UsbControl *control,
    uint8_t configuration, uint8_t alternativeSettings)
{
  if (!configuration)
  {
    usbDevSetConfigured(control->base, false);
  }
  else
  {
    if (!control->driver)
      return E_ERROR;

    const struct UsbDescriptor ** const root =
        usbDriverGetDescriptors(control->driver);
    const enum result res = traverseConfigArray(control, root,
        configuration, alternativeSettings);

    if (res != E_OK)
      return res;

    control->currentConfiguration = configuration;
    usbDevSetConfigured(control->base, true);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result traverseConfigArray(struct UsbControl *control,
    const struct UsbDescriptor **root, uint8_t configuration, uint8_t settings)
{
  //FIXME Choose default values
  uint8_t currentConfiguration = 0;
  uint8_t currentSettings = 0xFF;

  while (*root)
  {
    if ((*root)->descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
    {
      const struct UsbConfigurationDescriptor * const data =
          (const struct UsbConfigurationDescriptor *)(*root);

      currentConfiguration = data->configurationValue;
    }
    else if ((*root)->descriptorType == DESCRIPTOR_TYPE_INTERFACE)
    {
      const struct UsbInterfaceDescriptor * const data =
          (const struct UsbInterfaceDescriptor *)(*root);

      currentSettings = data->alternateSettings;
    }
    else if ((*root)->descriptorType == DESCRIPTOR_TYPE_ENDPOINT
        && currentConfiguration == configuration
        && currentSettings == settings)
    {
      const struct UsbEndpointDescriptor * const data =
          (const struct UsbEndpointDescriptor *)(*root);
      const uint16_t endpointSize = fromLittleEndian16(data->maxPacketSize);
      struct UsbEndpoint * const endpoint = usbDevAllocate(control->base,
          data->endpointAddress);

      if (!endpoint)
      {
        /* The endpoint is not allocated or the address is incorrect */
        return E_VALUE;
      }

      usbEpSetEnabled(endpoint, true, endpointSize);
    }

    ++root;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum result usbHandleStandardRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *buffer, uint16_t *length)
{
  switch (REQUEST_RECIPIENT_VALUE(packet->requestType))
  {
    case REQUEST_RECIPIENT_DEVICE:
      return handleStandardDeviceRequest(control, packet, buffer, length);

    case REQUEST_RECIPIENT_INTERFACE:
      return handleStandardInterfaceRequest(control, packet, buffer, length);

    case REQUEST_RECIPIENT_ENDPOINT:
      return handleStandardEndpointRequest(control, packet, buffer, length);

    default:
      return E_INVALID;
  }
}
