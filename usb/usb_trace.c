/*
 * usb_trace.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/timer.h>
#include <halm/usb/usb_trace.h>
#include <xcore/interface.h>
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define CONFIG_TRACE_BUFFER_SIZE 80
/*----------------------------------------------------------------------------*/
static struct Interface *traceSerial = NULL;
static struct Timer *traceTimer = NULL;
static char traceBuffer[CONFIG_TRACE_BUFFER_SIZE];
/*----------------------------------------------------------------------------*/
enum Result usbTraceInit(void *serial, void *timer)
{
  traceSerial = serial;
  traceTimer = timer;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void usbTraceDeinit(void)
{
  traceSerial = NULL;
  traceTimer = NULL;
}
/*----------------------------------------------------------------------------*/
void usbTrace(const char *format, ...)
{
  if (traceSerial == NULL)
    return;

  va_list arguments;
  int length;

  if (traceTimer != NULL)
  {
    const uint32_t timerValue = timerGetValue(traceTimer);

    length = sprintf(traceBuffer, "[%"PRIu32"] ", timerValue);
    assert(length >= 0);
    ifWrite(traceSerial, traceBuffer, (size_t)length);
  }

  va_start(arguments, format);
  length = vsnprintf(traceBuffer, CONFIG_TRACE_BUFFER_SIZE - 2,
      format, arguments);
  va_end(arguments);

  assert(length >= 0);
  memcpy(traceBuffer + length, "\r\n", 2);
  ifWrite(traceSerial, traceBuffer, (size_t)(length + 2));
}
