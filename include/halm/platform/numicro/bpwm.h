/*
 * halm/platform/numicro/bpwm.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BPWM_H_
#define HALM_PLATFORM_NUMICRO_BPWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/bpwm_base.h>
#include <halm/pwm.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const BpwmUnit;

struct BpwmUnitConfig
{
  /** Mandatory: timer frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: enable center-aligned mode. */
  bool centered;
};

struct BpwmUnit
{
  struct BpwmUnitBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Cycle width measured in timer ticks */
  uint32_t resolution;
  /* Used channels mask */
  uint8_t used;
  /* Enable center-aligned mode */
  bool centered;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const Bpwm;

struct BpwmConfig
{
  /** Mandatory: peripheral unit. */
  struct BpwmUnit *parent;
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
  /** Optional: enable output inversion. */
  bool inversion;
};

struct Bpwm
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct BpwmUnit *unit;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *bpwmCreate(void *, PinNumber, bool);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_BPWM_H_ */
