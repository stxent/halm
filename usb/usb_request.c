/*
 * usb_request.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
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
void usbRequestInit(struct UsbRequest *request, void *buffer, uint16_t capacity,
    void (*callback)(void *, struct UsbRequest *, enum UsbRequestStatus),
    void *callbackArgument)
{
  request->capacity = capacity;
  request->length = 0;
  request->callback = callback;
  request->callbackArgument = callbackArgument;
  request->buffer = buffer;
}
/*----------------------------------------------------------------------------*/
enum result usbExtractDescriptorData(const void *driver, uint16_t keyword,
    void *response, uint16_t *responseLength, uint16_t maxResponseLength)
{
  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);
  const usbDescriptorFunctor *entry = findEntry(driver,
      descriptorType, descriptorIndex);

  if (!entry)
  {
    usbTrace("control: descriptor %u:%u not found",
        descriptorType, descriptorIndex);
    return E_INVALID;
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
