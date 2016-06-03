/*
 * halm/usb/usb_requests.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_REQUESTS_H_
#define HALM_USB_USB_REQUESTS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <halm/usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
struct UsbControl;
/*----------------------------------------------------------------------------*/
enum result usbHandleStandardRequest(struct UsbControl *,
    const struct UsbSetupPacket *, uint8_t *, uint16_t *, uint16_t);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_REQUESTS_H_ */
