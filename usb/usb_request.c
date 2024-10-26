/*
 * usb_request.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *findEntry(const void *, uint8_t,
    unsigned int);
/*----------------------------------------------------------------------------*/
static const UsbDescriptorFunctor *findEntry(const void *driver, uint8_t type,
    unsigned int index)
{
  const UsbDescriptorFunctor *descriptor = usbDriverDescribe(driver);

  if (descriptor != NULL)
  {
    while (*descriptor != NULL)
    {
      struct UsbDescriptor header;

      (*descriptor)(driver, &header, NULL);
      if (header.descriptorType == type && !index--)
        return descriptor;
      ++descriptor;
    }
  }

  return NULL;
}
/*----------------------------------------------------------------------------*/
void usbRequestInit(struct UsbRequest *request, void *buffer, uint16_t capacity,
    UsbRequestCallback callback, void *argument)
{
  request->capacity = capacity;
  request->length = 0;
  request->callback = callback;
  request->argument = argument;
  request->buffer = buffer;
}
/*----------------------------------------------------------------------------*/
enum Result usbExtractDescriptorData(const void *driver, uint16_t keyword,
    void *response, uint16_t *responseLength, uint16_t maxResponseLength)
{
  static const uintptr_t lengthOffset =
      offsetof(struct UsbConfigurationDescriptor, totalLength);

  const uint8_t descriptorIndex = DESCRIPTOR_INDEX(keyword);
  const uint8_t descriptorType = DESCRIPTOR_TYPE(keyword);
  const UsbDescriptorFunctor *entry = findEntry(driver,
      descriptorType, descriptorIndex);

  if (entry == NULL)
  {
    usbTrace("control: descriptor %u:%u not found",
        descriptorType, descriptorIndex);
    return E_INVALID;
  }

  struct UsbDescriptor header;
  uint16_t length;

  (*entry)(driver, &header, NULL);

  if (header.descriptorType == DESCRIPTOR_TYPE_CONFIGURATION)
  {
    /* Extract total length from the Configuration Descriptor */
    (*entry)(driver, &header, response);
    memcpy(&length, (const uint8_t *)response + lengthOffset, sizeof(length));
    length = fromLittleEndian16(length);
  }
  else
    length = header.length;

  assert(length > 0 && length <= maxResponseLength);
  (void)maxResponseLength;

  assert(responseLength != NULL);
  *responseLength = length;

  while (length > 0 && *entry != NULL)
  {
    assert(((*entry)(driver, &header, NULL), length >= header.length));
    (*entry)(driver, &header, response);

    response = (void *)((uintptr_t)response + header.length);
    length -= (int32_t)header.length;

    ++entry;
  }

  return E_OK;
}
