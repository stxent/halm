/*
 * usb_trace.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/usb/usb_trace.h>
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_TRACE_BUFFER_SIZE 80
/*----------------------------------------------------------------------------*/
static struct Interface *traceInterface = 0;
static struct Timer *traceTimer = 0;
static char traceBuffer[CONFIG_TRACE_BUFFER_SIZE];
/*----------------------------------------------------------------------------*/
enum Result usbTraceInit(struct Interface *interface, struct Timer *timer)
{
  traceInterface = interface;
  traceTimer = timer;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbTraceDeinit(void)
{
  traceInterface = 0;
  traceTimer = 0;
}
/*----------------------------------------------------------------------------*/
void usbTrace(const char *format, ...)
{
  if (!traceInterface)
    return;

  va_list arguments;
  int length;

  if (traceTimer)
  {
    const uint32_t timerValue = timerGetValue(traceTimer);

    length = sprintf(traceBuffer, "[%"PRIu32"] ", timerValue);
    assert(length >= 0);
    ifWrite(traceInterface, traceBuffer, (size_t)length);
  }

  va_start(arguments, format);
  length = vsnprintf(traceBuffer, CONFIG_TRACE_BUFFER_SIZE - 2,
      format, arguments);
  va_end(arguments);

  assert(length >= 0);
  memcpy(traceBuffer + length, "\r\n", 2);
  ifWrite(traceInterface, traceBuffer, (size_t)(length + 2));
}
