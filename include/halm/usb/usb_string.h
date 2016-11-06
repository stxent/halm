/*
 * halm/usb/usb_string.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_STRING_H_
#define HALM_USB_USB_STRING_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <halm/usb/usb_langid.h>
/*----------------------------------------------------------------------------*/
struct UsbDescriptor;

typedef void (*usbStringFunctor)(void *, enum usbLangId, struct UsbDescriptor *,
    void *);
/*----------------------------------------------------------------------------*/
enum usbStringType
{
  USB_STRING_HEADER,
  USB_STRING_VENDOR,
  USB_STRING_PRODUCT,
  USB_STRING_SERIAL,
  USB_STRING_CUSTOM
};
/*----------------------------------------------------------------------------*/
struct UsbString
{
  usbStringFunctor functor;
  void *argument;
  uint8_t type;
};
/*----------------------------------------------------------------------------*/
struct UsbString usbStringBuild(usbStringFunctor, void *, enum usbStringType);
void usbStringHeader(struct UsbDescriptor *, void *, enum usbLangId);
void usbStringMultiHeader(struct UsbDescriptor *, void *,
    const enum usbLangId *, size_t);
void usbStringWrap(struct UsbDescriptor *, void *, const char *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_STRING_H_ */
