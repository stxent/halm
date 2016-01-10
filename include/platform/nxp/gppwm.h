/*
 * platform/nxp/gppwm.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPPWM_H_
#define PLATFORM_NXP_GPPWM_H_
/*----------------------------------------------------------------------------*/
#include <pwm.h>
#include <spinlock.h>
#include <platform/nxp/gppwm_base.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpPwmUnit;
extern const struct PwmClass * const GpPwm;
/*----------------------------------------------------------------------------*/
struct GpPwmUnitConfig
{
  /** Mandatory: switching frequency. */
  uint32_t frequency;
  /** Mandatory: cycle resolution. */
  uint32_t resolution;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpPwmUnit
{
  struct GpPwmUnitBase base;

  /* Cycle width measured in timer ticks */
  uint32_t resolution;
  /* Access protection for match registers state */
  spinlock_t spinlock;
  /* Match blocks currently in use */
  uint8_t matches;
};
/*----------------------------------------------------------------------------*/
struct GpPwmConfig
{
  /** Mandatory: peripheral unit. */
  struct GpPwmUnit *parent;
  /** Optional: initial duration in timer ticks. */
  uint32_t duration;
  /** Mandatory: pin used as an output for modulated signal. */
  pinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct GpPwm
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct GpPwmUnit *unit;
  /* Pointer to a match register */
  volatile uint32_t *value;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpPwmDoubleEdgeConfig
{
  /** Mandatory: peripheral unit. */
  struct GpPwmUnit *parent;
  /** Optional: initial leading edge time in timer ticks. */
  uint32_t leading;
  /** Optional: initial trailing edge time in timer ticks. */
  uint32_t trailing;
  /** Mandatory: pin used as an output for modulated signal. */
  pinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct GpPwmDoubleEdge
{
  struct Pwm base;

  /* Pointer to a parent unit */
  struct GpPwmUnit *unit;
  /* Pointer to a match register containing leading edge time */
  volatile uint32_t *leading;
  /* Pointer to a match register containing trailing edge time */
  volatile uint32_t *trailing;
  /* Number of the main match channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
void *gpPwmCreate(void *, pinNumber, uint32_t);
void *gpPwmCreateDoubleEdge(void *, pinNumber, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPPWM_H_ */
