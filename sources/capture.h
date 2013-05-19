/*
 * capture.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CAPTURE_H_
#define CAPTURE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "entity.h"
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
void captureSetCallback(void *, void (*)(void *), void *);
void captureSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
#endif /* CAPTURE_H_ */
