/*
 * usb_trace.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <halm/usb/usb_trace.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_TRACE_BUFFER_SIZE 80
/*----------------------------------------------------------------------------*/
static struct Interface *traceInterface = 0;
static char traceBuffer[CONFIG_TRACE_BUFFER_SIZE];
/*----------------------------------------------------------------------------*/
enum Result usbTraceInit(struct Interface *interface)
{
  traceInterface = interface;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbTraceDeinit(void)
{
  traceInterface = 0;
}
/*----------------------------------------------------------------------------*/
void usbTrace(const char *format, ...)
{
  if (!traceInterface)
    return;

  va_list arguments;
  int length;

  va_start(arguments, format);
  length = vsnprintf(traceBuffer, CONFIG_TRACE_BUFFER_SIZE - 2,
      format, arguments);
  va_end(arguments);

  memcpy(traceBuffer + length, "\r\n", 2);

  ifWrite(traceInterface, traceBuffer, (size_t)(length + 2));
}
