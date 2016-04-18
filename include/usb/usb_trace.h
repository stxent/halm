/*
 * usb/usb_trace.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_TRACE_H_
#define HALM_USB_USB_TRACE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
/*----------------------------------------------------------------------------*/
enum result usbTraceInit(struct Interface *);
void usbTraceDeinit();
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_USB_TRACE
void usbTrace(const char *, ...);
#else
#define usbTrace(...) do {} while (0)
#endif
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_TRACE_H_ */
