/*
 * capture.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "capture.h"
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
