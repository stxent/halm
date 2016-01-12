/*
 * usb.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
void usbRequestInit(struct UsbRequest *request, uint16_t capacity,
    void (*callback)(void *, struct UsbRequest *, enum usbRequestStatus),
    void *callbackArgument)
{
  request->base.capacity = capacity;
  request->base.length = 0;
  request->base.callback = callback;
  request->base.callbackArgument = callbackArgument;
}
