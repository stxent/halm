/*
 * halm/platform/bouffalo/pwm.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_PWM_H_
#define HALM_PLATFORM_BOUFFALO_PWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/pwm_base.h>
#include <halm/pwm.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const PwmUnit;

struct PwmUnitConfig
{
  /** Mandatory: timer frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Optional: timer interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct PwmUnit
{
  struct PwmBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Cycle width measured in timer ticks */
  uint32_t resolution;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const PwmSingleEdge;

struct PwmSingleEdgeConfig
{
  /** Mandatory: peripheral unit. */
  struct PwmUnit *parent;
  /** Mandatory: pin used as an output for the modulated signal. */
  PinNumber pin;
  /** Optional: enable output inversion. */
  bool inversion;
};

struct PwmSingleEdge
{
  struct Pwm base;

  /* Pointer to the parent unit */
  struct PwmUnit *unit;
  /* Enable output inversion */
  bool inversion;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const PwmDoubleEdge;

struct PwmDoubleEdgeConfig
{
  /** Mandatory: peripheral unit. */
  struct PwmUnit *parent;
  /** Mandatory: pin used as an output for the modulated signal. */
  PinNumber pin;
  /** Optional: enable output inversion. */
  bool inversion;
};

struct PwmDoubleEdge
{
  struct Pwm base;

  /* Pointer to the parent unit */
  struct PwmUnit *unit;
  /* Enable output inversion */
  bool inversion;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *pwmCreate(void *, PinNumber, bool);
void *pwmCreateDoubleEdge(void *, PinNumber, bool);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_PWM_H_ */
