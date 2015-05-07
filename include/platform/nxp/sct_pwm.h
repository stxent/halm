/*
 * platform/nxp/sct_pwm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SCT_PWM_H_
#define PLATFORM_NXP_SCT_PWM_H_
/*----------------------------------------------------------------------------*/
#include <pwm.h>
#include <platform/nxp/sct_base.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SctPwmUnit;
extern const struct PwmClass * const SctPwm;
/*----------------------------------------------------------------------------*/
struct SctPwmUnitConfig
{
  /** Mandatory: switching frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: timer part. */
  enum sctPart part;
};
/*----------------------------------------------------------------------------*/
struct SctPwmUnit
{
  struct SctBase parent;

  /* Timer frequency */
  uint32_t frequency;
  /* Cycle width measured in timer ticks */
  uint32_t resolution;
  /* Match channel used for counter reset */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
struct SctPwmConfig
{
  /** Mandatory: peripheral unit. */
  struct SctPwmUnit *parent;
  /** Optional: initial duration in timer ticks. */
  uint32_t duration;
  /** Mandatory: pin used as an output for modulated signal. */
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct SctPwm
{
  struct Pwm parent;

  /* Pointer to a parent unit */
  struct SctPwmUnit *unit;
  /* Output channel number */
  uint8_t channel;
  /* Event number */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
void *sctPwmCreate(void *, pin_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SCT_PWM_H_ */
