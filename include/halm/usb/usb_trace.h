/*
 * halm/usb/usb_trace.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_TRACE_H_
#define HALM_USB_USB_TRACE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result usbTraceInit(void *, void *);
void usbTraceDeinit(void);

#ifdef CONFIG_USB_TRACE
void usbTrace(const char *, ...);
#else
#  define usbTrace(...) do {} while (0)
#endif

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_TRACE_H_ */
