/*
 * platform/nxp/gppwm_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPPWM_BASE_H_
#define PLATFORM_NXP_GPPWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpPwmUnitBase;
/*----------------------------------------------------------------------------*/
struct GpPwmUnitBaseConfig
{
  uint8_t channel; /* Mandatory: modulator block */
};
/*----------------------------------------------------------------------------*/
struct GpPwmUnitBase
{
  struct Entity parent;

  void *reg;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t gpPwmGetClock(const struct GpPwmUnitBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPPWM_BASE_H_ */
