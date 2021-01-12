/*
 * halm/usb/usb_string.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_STRING_H_
#define HALM_USB_USB_STRING_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb_langid.h>
#include <xcore/helpers.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
struct UsbDescriptor;

typedef int UsbStringIndex;
typedef void (*UsbStringFunctor)(const void *, enum UsbLangId,
    struct UsbDescriptor *, void *);

enum UsbStringType
{
  USB_STRING_CONFIGURATION,
  USB_STRING_CUSTOM,
  USB_STRING_HEADER,
  USB_STRING_INTERFACE,
  USB_STRING_PRODUCT,
  USB_STRING_SERIAL,
  USB_STRING_VENDOR
};

struct UsbString
{
  /**
   * Function for string conversion to the expected format or
   * for a language identifier list generation.
   */
  UsbStringFunctor functor;
  /**
   * Pointer to an UTF-8 encoded string for generic string descriptors or
   * a null pointer for header descriptors.
   */
  const void *argument;
  /**
   * The USB string index used in other descriptors. Can be set to zero for
   * automatic index allocation.
   */
  uint8_t index;
  /**
   * Configuration or interface number. Applicable only for
   * Configuration and Interface strings.
   */
  uint8_t number;
  /** String type. */
  uint8_t type;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct UsbString usbStringBuild(UsbStringFunctor, const void *,
    enum UsbStringType, unsigned int);
struct UsbString usbStringBuildCustom(UsbStringFunctor, const void *,
    UsbStringIndex);
void usbStringHeader(struct UsbDescriptor *, void *, enum UsbLangId);
void usbStringMultiHeader(struct UsbDescriptor *, void *,
    const enum UsbLangId *, size_t);
void usbStringWrap(struct UsbDescriptor *, void *, const char *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_STRING_H_ */
