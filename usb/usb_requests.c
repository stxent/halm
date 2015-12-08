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
    uint16_t, uint16_t, uint8_t *, uint16_t *, uint16_t);
static enum result handleStandardDeviceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum result handleStandardEndpointRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
static enum result handleStandardInterfaceRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
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
    uint16_t keyword, uint16_t language, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
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
    if (2 + (stringLength << 1) > maxResponseLength)
      return E_VALUE;

    uToUtf16((char16_t *)(response + 2), data, stringLength + 1);
    response[0] = 2 + (stringLength << 1);
    response[1] = DESCRIPTOR_TYPE_STRING;
    *responseLength = response[0];
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

  if (chunkLength > maxResponseLength)
    return E_VALUE;

  if (chunkLength)
    *responseLength = chunkLength;

  while (*entry && chunkLength)
  {
    const uint8_t entryLength = (*entry)->length;

    memcpy(response, *entry, entryLength);
    response += entryLength;
    chunkLength -= entryLength;

    ++entry;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result handleStandardDeviceRequest(struct UsbControl *control,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Bit 0: self-powered */
      /* bit 1: remote wakeup */
      response[0] = 0; //FIXME
      response[1] = 0;
      *responseLength = 2;
      break;

    case REQUEST_SET_ADDRESS:
      usbDevSetAddress(control->owner, packet->value);
      usbTrace("requests: set address %d", packet->value);
      *responseLength = 0;
      break;

    case REQUEST_GET_DESCRIPTOR:
    {
      const struct UsbDescriptor ** const root =
          usbDriverGetDescriptors(control->driver);

      usbTrace("requests: get descriptor %d:%d, length %u",
          DESCRIPTOR_TYPE(packet->value), DESCRIPTOR_INDEX(packet->value),
          packet->length);

      return getDescriptorData(root, packet->value, packet->index, response,
          responseLength, maxResponseLength);
    }

    case REQUEST_GET_CONFIGURATION:
      response[0] = usbDevGetConfiguration(control->owner);
      *responseLength = 1;
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
        *responseLength = 0;
        break;
      }
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_REMOTE_WAKEUP)
      {
        // TODO Put DEVICE_REMOTE_WAKEUP code here
      }
      if (packet->value == FEATURE_TEST_MODE)
      {
        // TODO Put TEST_MODE code here
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
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength,
    uint16_t maxResponseLength __attribute__((unused)))
{
  struct UsbEndpoint *endpoint = usbDevAllocate(control->owner, packet->index);

  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Endpoint is halted or not */
      response[0] = (uint8_t)usbEpIsStalled(endpoint);
      response[1] = 0; /* Must be set to zero */
      *responseLength = 2;
      return E_OK;

    case REQUEST_CLEAR_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Clear halt by unstalling */
        usbEpSetStalled(endpoint, false);
        *responseLength = 0;
        usbTrace("requests: unstall endpoint %02X", packet->index);
        return E_OK;
      }
      break;

    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
      {
        /* Set halt by stalling */
        usbEpSetStalled(endpoint, true);
        *responseLength = 0;
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
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Two bytes are reserved for future use and must be set to zero */
      response[0] = 0;
      response[1] = 0;
      *responseLength = 2;
      break;

    case REQUEST_GET_INTERFACE: // TODO use bNumInterfaces
      response[0] = 0;
      *responseLength = 1;
      break;

    case REQUEST_SET_INTERFACE: // TODO use bNumInterfaces
      usbTrace("requests: set interface %d", packet->value);
      if (packet->value != 0)
        return E_ERROR; //FIXME
      *responseLength = 0;
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
    usbDevSetConfiguration(control->owner, 0);
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

    usbDevSetConfiguration(control->owner, configuration);

    if (control->driver)
    {
      /* Notify the driver */
      usbDriverUpdateStatus(control->driver, DEVICE_STATUS_RESET);
    }
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
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength, uint16_t maxResponseLength)
{
  const uint8_t type = REQUEST_TYPE_VALUE(packet->requestType);

  if (type != REQUEST_TYPE_STANDARD)
    return E_INVALID;

  switch (REQUEST_RECIPIENT_VALUE(packet->requestType))
  {
    case REQUEST_RECIPIENT_DEVICE:
      return handleStandardDeviceRequest(control, packet, response,
          responseLength, maxResponseLength);

    case REQUEST_RECIPIENT_INTERFACE:
      return handleStandardInterfaceRequest(control, packet, response,
          responseLength, maxResponseLength);

    case REQUEST_RECIPIENT_ENDPOINT:
      return handleStandardEndpointRequest(control, packet, response,
          responseLength, maxResponseLength);

    default:
      return E_INVALID;
  }
}
