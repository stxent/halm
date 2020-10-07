/*
 * halm/capture.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_CAPTURE_H_
#define HALM_CAPTURE_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
#include <xcore/entity.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct CaptureClass
{
  CLASS_HEADER

  void (*enable)(void *);
  void (*disable)(void *);

  void (*setCallback)(void *, void (*)(void *), void *);
  uint32_t (*getValue)(const void *);
};

struct Capture
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Start event capture.
 * @param capture Pointer to a Capture object.
 */
static inline void captureEnable(void *capture)
{
  ((const struct CaptureClass *)CLASS(capture))->enable(capture);
}

/**
 * Stop event capture.
 * @param capture Pointer to a Capture object.
 */
static inline void captureDisable(void *capture)
{
  ((const struct CaptureClass *)CLASS(capture))->disable(capture);
}

/**
 * Set callback function for capture event.
 * @param capture Pointer to a Capture object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void captureSetCallback(void *capture, void (*callback)(void *),
    void *argument)
{
  ((const struct CaptureClass *)CLASS(capture))->setCallback(capture, callback,
      argument);
}

/**
 * Get captured value.
 * @param capture Pointer to a Capture object.
 * @return Value of the counter measured in timer ticks.
 */
static inline uint32_t captureGetValue(const void *capture)
{
  return ((const struct CaptureClass *)CLASS(capture))->getValue(capture);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_CAPTURE_H_ */
