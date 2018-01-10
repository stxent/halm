/*
 * halm/usb/usb_string.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_STRING_H_
#define HALM_USB_USB_STRING_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/helpers.h>
#include <halm/usb/usb_langid.h>
/*----------------------------------------------------------------------------*/
struct UsbDescriptor;

typedef void (*usbStringFunctor)(void *, enum UsbLangId,
    struct UsbDescriptor *, void *);

enum UsbStringType
{
  USB_STRING_HEADER,
  USB_STRING_VENDOR,
  USB_STRING_PRODUCT,
  USB_STRING_SERIAL,
  USB_STRING_CUSTOM
};

struct UsbString
{
  usbStringFunctor functor;
  void *argument;
  uint8_t type;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct UsbString usbStringBuild(usbStringFunctor, void *, enum UsbStringType);
void usbStringHeader(struct UsbDescriptor *, void *, enum UsbLangId);
void usbStringMultiHeader(struct UsbDescriptor *, void *,
    const enum UsbLangId *, size_t);
void usbStringWrap(struct UsbDescriptor *, void *, const char *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_STRING_H_ */