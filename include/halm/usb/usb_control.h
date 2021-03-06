/*
 * halm/usb/usb_control.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_CONTROL_H_
#define HALM_USB_USB_CONTROL_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb_string.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbControl;

struct UsbControl;
struct UsbDevice;

struct UsbControlConfig
{
  /** Mandatory: parent device. */
  struct UsbDevice *parent;
  /** Mandatory: Vendor Identifier. */
  uint16_t vid;
  /** Mandatory: Product Identifier. */
  uint16_t pid;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result usbControlBindDriver(struct UsbControl *, void *);
void usbControlUnbindDriver(struct UsbControl *);
void usbControlNotify(struct UsbControl *, unsigned int);
void usbControlSetPower(struct UsbControl *, unsigned int);
UsbStringIndex usbControlStringAppend(struct UsbControl *, struct UsbString);
UsbStringIndex usbControlStringFind(struct UsbControl *, enum UsbStringType,
    unsigned int);
void usbControlStringErase(struct UsbControl *, struct UsbString);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_H_ */
