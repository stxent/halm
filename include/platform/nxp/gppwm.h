/*
 * platform/nxp/gppwm.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPPWM_H_
#define GPPWM_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <pwm.h>
#include "gppwm_base.h"
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *GpPwmUnit;
extern const struct PwmClass *GpPwm;
/*----------------------------------------------------------------------------*/
struct GpPwmUnitConfig
{
  uint32_t frequency; /* Mandatory: cycle frequency */
  uint32_t resolution; /* Mandatory: cycle resolution */
  uint8_t channel; /* Mandatory: timer block */
};
/*----------------------------------------------------------------------------*/
struct GpPwmUnit
{
  struct GpPwmUnitBase parent;

  uint32_t resolution;
  uint8_t matches; /* Match blocks currently in use */
};
/*----------------------------------------------------------------------------*/
struct GpPwmConfig
{
  struct GpPwmUnit *parent; /* Mandatory: peripheral unit */
  uint32_t duration; /* Optional: initial duration in timer ticks */
  gpio_t pin; /* Mandatory: pin used as output for modulated signal */
};
/*----------------------------------------------------------------------------*/
struct GpPwm
{
  struct Pwm parent;

  /* Pointer to a parent unit */
  struct GpPwmUnit *unit;
  /* Pointer to a match register */
  uint32_t *value;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpPwmDoubleEdgeConfig
{
  struct GpPwmUnit *parent; /* Mandatory: peripheral unit */
  uint32_t leading; /* Optional: initial leading edge time in timer ticks */
  uint32_t trailing; /* Optional: initial trailing edge time in timer ticks */
  gpio_t pin; /* Mandatory: pin used as output for modulated signal */
};
/*----------------------------------------------------------------------------*/
struct GpPwmDoubleEdge
{
  struct Pwm parent;

  /* Pointer to a parent unit */
  struct GpPwmUnit *unit;
  /* Pointer to a match register */
  uint32_t *value;
  /* Match channel number */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
void *gpPwmCreate(void *, gpio_t, uint32_t);
void *gpPwmCreateDoubleEdge(void *, gpio_t, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* GPPWM_H_ */
