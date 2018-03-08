/*
 * halm/platform/nxp/gptimer_pwm.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPTIMER_PWM_H_
#define HALM_PLATFORM_NXP_GPTIMER_PWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/nxp/gptimer_base.h>
#include <halm/pwm.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpTimerPwmUnit;

struct GpTimerPwmUnitConfig
{
  /** Mandatory: switching frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct GpTimerPwmUnit
{
  struct GpTimerBase base;

  /* Desired timer frequency */
  uint32_t frequency;
  /* Cycle width measured in timer ticks */
  uint32_t resolution;
  /* Match block used for period configuration */
  uint8_t limiter;
  /* Match blocks currently in use */
  uint8_t matches;
};
/*----------------------------------------------------------------------------*/
extern const struct PwmClass * const GpTimerPwm;

struct GpTimerPwmConfig
{
  /** Mandatory: peripheral unit. */
  struct GpTimerPwmUnit *parent;
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
};

struct GpTimerPwm
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct GpTimerPwmUnit *unit;
  /* Pointer to a match register */
  volatile uint32_t *value;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void *gpTimerPwmCreate(void *, PinNumber);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPTIMER_PWM_H_ */
