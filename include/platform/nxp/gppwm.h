/*
 * platform/nxp/gppwm.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPPWM_H_
#define GPPWM_H_
/*----------------------------------------------------------------------------*/
#include <pwm.h>
#include "gppwm_base.h"
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
  struct GpPwmUnitBase parent;

  /* Cycle width measured in timer ticks */
  uint32_t resolution;
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
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct GpPwm
{
  struct Pwm parent;

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
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct GpPwmDoubleEdge
{
  struct Pwm parent;

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
void *gpPwmCreate(void *, pin_t, uint32_t);
void *gpPwmCreateDoubleEdge(void *, pin_t, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* GPPWM_H_ */
