/*
 * capture.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <capture.h>
/*----------------------------------------------------------------------------*/
/**
 * Create event capture object, associated with capture block.
 * By default event capture is stopped.
 * @param timer Pointer to Timer object.
 * @param pin GPIO pin used as source input.
 * @param mode Capture mode.
 * @return Pointer to new Capture object on success or zero on error.
 */
void *captureCreate(void *block, struct Gpio pin, enum captureMode mode)
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
void captureSetCallback(void *channel, void (*callback)(void *),
    void *parameters)
{
  ((struct CaptureClass *)CLASS(channel))->setCallback(channel, callback,
      parameters);
}
/*----------------------------------------------------------------------------*/
/**
 * Start or stop event capture.
 * @param channel Pointer to Capture object.
 * @param state Capture state, @b true to start or @b false to stop
 * event capture.
 */
void captureSetEnabled(void *channel, bool state)
{
  ((struct CaptureClass *)CLASS(channel))->setEnabled(channel, state);
}
