/*
 * halm/platform/lpc/gptimer_capture.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_CAPTURE_H_
#define HALM_PLATFORM_LPC_GPTIMER_CAPTURE_H_
/*----------------------------------------------------------------------------*/
#include <halm/capture.h>
#include <halm/platform/lpc/gptimer_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const GpTimerCaptureUnit;

struct GpTimerCapture;

struct GpTimerCaptureUnitConfig
{
  /** Mandatory: timer frequency. */
  uint32_t frequency;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct GpTimerCaptureUnit
{
  struct GpTimerBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Registered capture handlers */
  struct GpTimerCapture *instances[4];
  /* Desired timer frequency */
  uint32_t frequency;
};
/*----------------------------------------------------------------------------*/
extern const struct CaptureClass * const GpTimerCapture;

struct GpTimerCaptureConfig
{
  /** Mandatory: peripheral unit. */
  struct GpTimerCaptureUnit *parent;
  /** Mandatory: capture trigger. */
  enum InputEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum PinPull pull;
  /** Mandatory: pin used as an input. */
  PinNumber pin;
};

struct GpTimerCapture
{
  struct Capture base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to a parent unit */
  struct GpTimerCaptureUnit *unit;
  /* Pointer to a capture register */
  const volatile uint32_t *value;
  /* Capture trigger */
  enum InputEvent event;
  /* Channel identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *gpTimerCaptureCreate(void *, PinNumber, enum InputEvent, enum PinPull);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPTIMER_CAPTURE_H_ */
