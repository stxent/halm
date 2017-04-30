/*
 * halm/usb/usb_request.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_REQUEST_H_
#define HALM_USB_USB_REQUEST_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
struct UsbRequest
{
  uint16_t capacity;
  uint16_t length;

  void (*callback)(void *, struct UsbRequest *, enum UsbRequestStatus);
  void *callbackArgument;

  uint8_t *buffer;
};
/*----------------------------------------------------------------------------*/
void usbRequestInit(struct UsbRequest *, void *, uint16_t,
    void (*)(void *, struct UsbRequest *, enum UsbRequestStatus), void *);
/*----------------------------------------------------------------------------*/
enum result usbExtractDescriptorData(const void *, uint16_t, void *, uint16_t *,
    uint16_t);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_REQUEST_H_ */
