/*
 * halm/usb/usb_control.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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
void usbControlEvent(struct UsbControl *, unsigned int);
void usbControlSetPower(struct UsbControl *, uint16_t);
enum Result usbControlStringAppend(struct UsbControl *, struct UsbString);
void usbControlStringErase(struct UsbControl *, struct UsbString);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_H_ */
