/*
 * usb_string.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_string.h>
#include <xcore/memory.h>
#include <xcore/unicode.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UsbString usbStringBuild(UsbStringFunctor functor, const void *argument,
    enum UsbStringType type)
{
  return (struct UsbString){
    .functor = functor,
    .argument = argument,
    .type = type
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
    struct UsbStringDescriptor * const descriptor =
        (struct UsbStringDescriptor *)payload;

    descriptor->langid[0] = toLittleEndian16(langid);
  }
}
/*----------------------------------------------------------------------------*/
void usbStringMultiHeader(struct UsbDescriptor *header, void *payload,
    const enum UsbLangId *languages, size_t languageCount)
{
  assert(languageCount >= 1);

  header->length = sizeof(struct UsbStringDescriptor)
      + sizeof(uint16_t) * languageCount;
  header->descriptorType = DESCRIPTOR_TYPE_STRING;

  if (payload)
  {
    struct UsbStringDescriptor * const descriptor =
        (struct UsbStringDescriptor *)payload;

    for (size_t index = 0; index < languageCount; ++index)
      descriptor->langid[index] = toLittleEndian16(languages[index]);
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
    struct UsbDescriptor * const descriptor = payload;

    uToUtf16((char16_t *)descriptor->data, text, textLength + 1);
  }
}
