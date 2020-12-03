/*
 * halm/usb/usb_request.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_USB_REQUEST_H_
#define HALM_USB_USB_REQUEST_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
typedef void (*UsbRequestCallback)(void *, struct UsbRequest *,
    enum UsbRequestStatus);

struct UsbRequest
{
  uint16_t capacity;
  uint16_t length;

  UsbRequestCallback callback;
  void *callbackArgument;

  uint8_t *buffer;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void usbRequestInit(struct UsbRequest *, void *, uint16_t,
    UsbRequestCallback, void *);

enum Result usbExtractDescriptorData(const void *, uint16_t,
    void *, uint16_t *, uint16_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_REQUEST_H_ */
