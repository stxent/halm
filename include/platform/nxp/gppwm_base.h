/*
 * platform/nxp/gppwm_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPPWM_BASE_H_
#define GPPWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *GpPwmUnitBase;
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
uint32_t gpPwmGetClock(struct GpPwmUnitBase *);
/*----------------------------------------------------------------------------*/
#endif /* GPPWM_BASE_H_ */
