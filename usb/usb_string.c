/*
 * usb_string.c
 * Copyright (C) 2016, 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/usb_control_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_string.h>
#include <xcore/memory.h>
#include <xcore/unicode.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define MAX_STRING_LENGTH \
    (MIN(STRING_DESCRIPTOR_TEXT_LIMIT, DATA_BUFFER_SIZE - 2) / 2)
/*----------------------------------------------------------------------------*/
struct UsbString usbStringBuild(UsbStringFunctor functor, const void *argument,
    enum UsbStringType type, unsigned int number)
{
  assert(!(type == USB_STRING_CONFIGURATION || type == USB_STRING_INTERFACE)
      || number == 0);

  return (struct UsbString){
      .functor = functor,
      .argument = argument,
      .index = 0,
      .number = (uint8_t)number,
      .type = (uint8_t)type
  };
}
/*----------------------------------------------------------------------------*/
struct UsbString usbStringBuildCustom(UsbStringFunctor functor,
    const void *argument, UsbStringIndex index)
{
  return (struct UsbString){
      .functor = functor,
      .argument = argument,
      .index = (uint8_t)index,
      .number = 0,
      .type = USB_STRING_CUSTOM
  };
}
/*----------------------------------------------------------------------------*/
void usbStringHeader(struct UsbDescriptor *header, void *payload,
    enum UsbLangId langid)
{
  const struct UsbDescriptor descriptor = {
      .length = sizeof(struct UsbDescriptor) + sizeof(uint16_t),
      .descriptorType = DESCRIPTOR_TYPE_STRING
  };

  memcpy(header, &descriptor, sizeof(descriptor));

  if (payload != NULL)
  {
    const uint16_t value = toLittleEndian16(langid);
    uint8_t *buffer = payload;

    memcpy(buffer, &descriptor, sizeof(descriptor));
    memcpy(buffer + sizeof(descriptor), &value, sizeof(value));
    buffer += sizeof(descriptor);
  }
}
/*----------------------------------------------------------------------------*/
void usbStringMultiHeader(struct UsbDescriptor *header, void *payload,
    const enum UsbLangId *languages, size_t count)
{
  assert(count >= 1 && count <= MAX_STRING_LENGTH);

  const struct UsbDescriptor descriptor = {
      .length = sizeof(struct UsbDescriptor) + sizeof(uint16_t) * count,
      .descriptorType = DESCRIPTOR_TYPE_STRING
  };

  memcpy(header, &descriptor, sizeof(descriptor));

  if (payload != NULL)
  {
    uint8_t *buffer = payload;

    memcpy(buffer, &descriptor, sizeof(descriptor));
    buffer += sizeof(descriptor);

    for (size_t index = 0; index < count; ++index)
    {
      const uint16_t value = toLittleEndian16(languages[index]);

      memcpy(buffer, &value, sizeof(value));
      buffer += sizeof(value);
    }
  }
}
/*----------------------------------------------------------------------------*/
void usbStringWrap(struct UsbDescriptor *header, void *payload,
    const char *text)
{
  const size_t length = uLengthToUtf16(text);
  assert(length + 1 <= MAX_STRING_LENGTH);

  const struct UsbDescriptor descriptor = {
      .length = sizeof(struct UsbDescriptor) + sizeof(char16_t) * length,
      .descriptorType = DESCRIPTOR_TYPE_STRING
  };

  memcpy(header, &descriptor, sizeof(descriptor));

  if (payload != NULL)
  {
    char16_t converted[MAX_STRING_LENGTH];
    uint8_t *buffer = payload;

    uToUtf16(converted, text, length + 1);
    memcpy(buffer, &descriptor, sizeof(descriptor));
    memcpy(buffer + sizeof(descriptor), converted, sizeof(char16_t) * length);
  }
}
