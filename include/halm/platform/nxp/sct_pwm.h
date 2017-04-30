/*
 * halm/platform/nxp/sct_pwm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SCT_PWM_H_
#define HALM_PLATFORM_NXP_SCT_PWM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/nxp/sct_base.h>
#include <halm/pwm.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SctPwmUnit;
extern const struct PwmClass * const SctPwm;
extern const struct PwmClass * const SctPwmDoubleEdge;
/*----------------------------------------------------------------------------*/
struct SctPwmUnitConfig
{
  /** Mandatory: switching frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Optional: timer part. */
  enum SctPart part;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct SctPwmUnit
{
  struct SctBase base;

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
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct SctPwm
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct SctPwmUnit *unit;
  /* Output channel number */
  uint8_t channel;
  /* Event number */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
struct SctPwmDoubleEdgeConfig
{
  /** Mandatory: peripheral unit. */
  struct SctPwmUnit *parent;
  /** Mandatory: pin used as an output for modulated signal. */
  PinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct SctPwmDoubleEdge
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct SctPwmUnit *unit;
  /* Output channel number */
  uint8_t channel;
  /* Event number for leading edge */
  uint8_t leadingEvent;
  /* Event number for trailing edge */
  uint8_t trailingEvent;
};
/*----------------------------------------------------------------------------*/
void *sctPwmCreate(void *, PinNumber);
void *sctPwmCreateDoubleEdge(void *, PinNumber);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SCT_PWM_H_ */
