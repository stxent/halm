/*
 * usb.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
static enum result reqInit(void *, const void *);
static void reqDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass reqTable = {
    .size = sizeof(struct UsbRequest),
    .init = reqInit,
    .deinit = reqDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UsbRequest = &reqTable;
/*----------------------------------------------------------------------------*/
static enum result reqInit(void *object, const void *configBase)
{
  const struct UsbRequestConfig * const config = configBase;
  struct UsbRequest * const request = object;

  request->buffer = malloc(config->size);
  if (!request->buffer)
    return E_MEMORY;

  request->capacity = config->size;
  request->length = 0;
  request->status = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void reqDeinit(void *object)
{
  struct UsbRequest * const request = object;

  free(request->buffer);
}
