/*
 * usb_string.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_string.h>
#include <xcore/memory.h>
#include <xcore/unicode.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UsbStringDescriptor
{
  uint8_t length;
  uint8_t descriptorType;
  uint16_t data[];
};

static_assert(offsetof(struct UsbStringDescriptor, data) ==
    offsetof(struct UsbDescriptor, data), "Incorrect type declaration");
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
  header->length = sizeof(struct UsbStringDescriptor) + sizeof(uint16_t);
  header->descriptorType = DESCRIPTOR_TYPE_STRING;

  if (payload)
  {
    struct UsbStringDescriptor * const descriptor = payload;
    descriptor->data[0] = toLittleEndian16(langid);
  }
}
/*----------------------------------------------------------------------------*/
void usbStringMultiHeader(struct UsbDescriptor *header, void *payload,
    const enum UsbLangId *languages, size_t count)
{
  assert(count >= 1);

  header->length = sizeof(struct UsbStringDescriptor)
      + sizeof(uint16_t) * count;
  header->descriptorType = DESCRIPTOR_TYPE_STRING;

  if (payload)
  {
    struct UsbStringDescriptor * const descriptor = payload;

    for (size_t index = 0; index < count; ++index)
      descriptor->data[index] = toLittleEndian16(languages[index]);
  }
}
/*----------------------------------------------------------------------------*/
void usbStringWrap(struct UsbDescriptor *header, void *payload,
    const char *text)
{
  const size_t textLength = uLengthToUtf16(text);

  header->length = sizeof(struct UsbDescriptor) + textLength * 2;
  header->descriptorType = DESCRIPTOR_TYPE_STRING;

  if (payload)
  {
    struct UsbStringDescriptor * const descriptor = payload;
    uToUtf16((char16_t *)descriptor->data, text, textLength + 1);
  }
}
