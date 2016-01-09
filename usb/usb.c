/*
 * usb.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
enum result usbRequestInit(struct UsbRequest *request, uint16_t size)
{
  request->buffer = malloc(size);
  if (!request->buffer)
    return E_MEMORY;

  request->callback = 0;
  request->capacity = size;
  request->length = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbRequestDeinit(struct UsbRequest *request)
{
  free(request->buffer);
}
/*----------------------------------------------------------------------------*/
void usbRequestCallback(struct UsbRequest *request,
    void (*callback)(void *, struct UsbRequest *, enum usbRequestStatus),
    void *argument)
{
  request->callbackArgument = argument;
  request->callback = callback;
}
