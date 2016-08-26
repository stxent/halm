/*
 * usb_requests.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_requests.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor *findEntry(const void *, uint8_t,
    unsigned int);
/*----------------------------------------------------------------------------*/
static const usbDescriptorFunctor *findEntry(const void *driver, uint8_t type,
    unsigned int offset)
{
  const usbDescriptorFunctor *descriptors = usbDriverDescribe(driver);
  struct UsbDescriptor header;

  while (*descriptors)
  {
    (*descriptors)(driver, &header, 0);

    if (header.descriptorType == type)
    {
      if (offset)
        --offset;
      else
        return descriptors;
    }
    ++descriptors;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
enum result usbExtractDescriptorData(const void *driver, uint16_t keyword,
    uint16_t language __attribute__((unused)),
    void *response, uint16_t *responseLength, uint16_t maxResponseLength)
{
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);
  const usbDescriptorFunctor *entry = findEntry(driver,
      descriptorType, descriptorIndex);

  if (!entry)
  {
    usbTrace("requests: descriptor %u:%u not found",
        descriptorType, descriptorIndex);
    return E_VALUE;
  }

  struct UsbDescriptor header;
  uint16_t length = 0;

  (*entry)(driver, &header, 0);

  if (header.descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
  {
    assert(header.length && header.length <= maxResponseLength);

    /* Extract total length from the Configuration Descriptor */
    memset(response, 0, sizeof(struct UsbConfigurationDescriptor));
    (*entry)(driver, response, response);
    length = ((const struct UsbConfigurationDescriptor *)response)->totalLength;
  }
  else
    length = header.length;

  assert(length && length <= maxResponseLength);
  (void)maxResponseLength;

  *responseLength = length;
  memset(response, 0, length);

  while (length && *entry)
  {
    (*entry)(driver, &header, 0);

    if (header.descriptorType != DESCRIPTOR_TYPE_INTERFACE_ASSOCIATION)
    {
      struct UsbDescriptor * const entryHeader = response;

      (*entry)(driver, entryHeader, response);
      response = (void *)((uintptr_t)response + entryHeader->length);
      length -= entryHeader->length;
    }
    ++entry;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbFillConfigurationDescriptor(const void *device, void *buffer)
{
  struct UsbConfigurationDescriptor * const descriptor = buffer;
  uint16_t maxPower;
  bool selfPowered, remoteWakeup;

  usbDevGetParameter(device, USB_MAX_POWER, &maxPower);
  usbDevGetParameter(device, USB_SELF_POWERED, &selfPowered);
  usbDevGetParameter(device, USB_REMOTE_WAKEUP, &remoteWakeup);

  descriptor->attributes = CONFIGURATION_DESCRIPTOR_DEFAULT;
  if (selfPowered)
    descriptor->attributes |= CONFIGURATION_DESCRIPTOR_SELF_POWERED;
  if (remoteWakeup)
    descriptor->attributes |= CONFIGURATION_DESCRIPTOR_REMOTE_WAKEUP;
  descriptor->maxPower = ((maxPower + 1) >> 1);
}
/*----------------------------------------------------------------------------*/
void usbFillDeviceDescriptor(const void *device, void *buffer)
{
  struct UsbDeviceDescriptor * const descriptor = buffer;
  uint16_t vid, pid;

  usbDevGetParameter(device, USB_VID, &vid);
  usbDevGetParameter(device, USB_PID, &pid);

  descriptor->idVendor = toLittleEndian16(vid);
  descriptor->idProduct = toLittleEndian16(pid);
}
/*----------------------------------------------------------------------------*/
enum result usbHandleDeviceRequest(void *driver, void *device,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
    {
      uint8_t status = 0;
      bool selfPowered, remoteWakeup;

      usbDevGetParameter(device, USB_SELF_POWERED, &selfPowered);
      usbDevGetParameter(device, USB_REMOTE_WAKEUP, &remoteWakeup);

      if (selfPowered)
        status |= STATUS_SELF_POWERED;
      if (remoteWakeup)
        status |= STATUS_REMOTE_WAKEUP;

      response[0] = status;
      response[1] = 0;
      *responseLength = 2;
      return E_OK;
    }

    case REQUEST_CLEAR_FEATURE:
    case REQUEST_SET_FEATURE:
    {
      if (packet->value != FEATURE_REMOTE_WAKEUP)
        return E_VALUE;

      const bool value = packet->request == REQUEST_SET_FEATURE;
      return usbDevSetParameter(device, USB_REMOTE_WAKEUP, &value);
    }

    case REQUEST_SET_ADDRESS:
      usbTrace("requests: set address %u", packet->value);

      usbDevSetAddress(device, packet->value);
      *responseLength = 0;
      return E_OK;

    case REQUEST_GET_CONFIGURATION:
      response[0] = 1;
      *responseLength = 1;
      return E_OK;

    case REQUEST_SET_CONFIGURATION:
      usbTrace("requests: set configuration %u", packet->value);

      if (packet->value == 1)
      {
        usbDriverEvent(driver, USB_DEVICE_EVENT_RESET);
        return E_OK;
      }
      else
        return E_VALUE;

    default:
      usbTrace("requests: unsupported device request 0x%02X", packet->request);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
enum result usbHandleEndpointRequest(void *device,
    const struct UsbSetupPacket *packet, uint8_t *response,
    uint16_t *responseLength)
{
  struct UsbEndpoint * const endpoint = usbDevCreateEndpoint(device,
      packet->index);

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
        return E_VALUE;

      usbTrace("requests: enable endpoint 0x%02X", packet->index);

      /* Clear halt by endpoint enabling */
      usbEpSetStalled(endpoint, false);
      *responseLength = 0;
      return E_OK;

    case REQUEST_SET_FEATURE:
      if (packet->value == FEATURE_ENDPOINT_HALT)
        return E_VALUE;

      /* Set halt by endpoint stalling */
      usbEpSetStalled(endpoint, true);
      *responseLength = 0;
      usbTrace("requests: stall endpoint 0x%02X", packet->index);
      return E_OK;

    default:
      usbTrace("requests: unsupported request 0x%02X to endpoint 0x%02X",
          packet->request, packet->index);
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
enum result usbHandleInterfaceRequest(const struct UsbSetupPacket *packet,
    uint8_t *response, uint16_t *responseLength)
{
  switch (packet->request)
  {
    case REQUEST_GET_STATUS:
      /* Two bytes are reserved for future use and must be set to zero */
      response[0] = response[1] = 0;
      *responseLength = 2;
      return E_OK;

    case REQUEST_GET_INTERFACE:
      response[0] = 0;
      *responseLength = 1;
      return E_OK;

    case REQUEST_SET_INTERFACE:
      usbTrace("requests: set interface %u", packet->value);

      /* Only one interface is supported by default */
      if (packet->value == 0)
      {
        *responseLength = 0;
        return E_OK;
      }
      else
        return E_VALUE;

    default:
      usbTrace("requests: unsupported interface request 0x%02X",
          packet->request);
      return E_INVALID;
  }
}
