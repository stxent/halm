/*
 * halm/platform/nxp/gppwm_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPPWM_BASE_H_
#define HALM_PLATFORM_NXP_GPPWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/entity.h>
#include <halm/irq.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpPwmUnitBase;

struct GpPwmUnitBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct GpPwmUnitBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t gpPwmGetClock(const struct GpPwmUnitBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPPWM_BASE_H_ */
