/*
 * capture.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CAPTURE_H_
#define CAPTURE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <entity.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct CaptureClass
{
  CLASS_HEADER

  void (*callback)(void *, void (*)(void *), void *);
  void (*setEnabled)(void *, bool);
  uint32_t (*value)(const void *);
};
/*----------------------------------------------------------------------------*/
struct Capture
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Set callback function for capture event.
 * @param capture Pointer to a Capture object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void captureCallback(void *capture, void (*callback)(void *),
    void *argument)
{
  ((const struct CaptureClass *)CLASS(capture))->callback(capture, callback,
      argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop event capture.
 * @param capture Pointer to a Capture object.
 * @param state Capture state: @b true to enable or @b false to disable.
 */
static inline void captureSetEnabled(void *capture, bool state)
{
  ((const struct CaptureClass *)CLASS(capture))->setEnabled(capture, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Get captured value.
 * @param capture Pointer to a Capture object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t captureValue(const void *capture)
{
  return ((const struct CaptureClass *)CLASS(capture))->value(capture);
}
/*----------------------------------------------------------------------------*/
#endif /* CAPTURE_H_ */
