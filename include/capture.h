/*
 * capture.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CAPTURE_H_
#define CAPTURE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
enum captureMode
{
  CAPTURE_RISING,
  CAPTURE_FALLIND,
  CAPTURE_TOGGLE
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct CaptureClass
{
  CLASS_HEADER

  /* Virtual functions */
  void (*setCallback)(void *, void (*)(void *), void *);
  void (*setEnabled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
struct Capture
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Start or stop event capture.
 * @param channel Pointer to Capture object.
 * @param state Capture state, @b true to start or @b false to stop
 * event capture.
 */
static inline void captureControl(void *channel, bool state)
{
  ((struct CaptureClass *)CLASS(channel))->setEnabled(channel, state);
}
/*----------------------------------------------------------------------------*/
/**
 * Create event capture object, associated with capture block.
 * By default event capture is stopped.
 * @param timer Pointer to Timer object.
 * @param pin GPIO pin used as source input.
 * @param mode Capture mode.
 * @return Pointer to new Capture object on success or zero on error.
 */
static inline void *captureCreate(void *block, struct Gpio pin,
    enum captureMode mode)
{
  return ((struct CaptureControllerClass *)CLASS(block))->create(controller,
      pin, mode);
}
/*----------------------------------------------------------------------------*/
/**
 * Set capture event callback.
 * @param channel Pointer to Capture object.
 * @param callback Callback function.
 * @param parameters Callback function parameters.
 */
static inline void captureSetCallback(void *channel, void (*callback)(void *),
    void *parameters)
{
  ((struct CaptureClass *)CLASS(channel))->setCallback(channel, callback,
      parameters);
}
/*----------------------------------------------------------------------------*/
#endif /* CAPTURE_H_ */
