/*
 * halm/platform/lpc/gppwm_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPPWM_BASE_H_
#define HALM_PLATFORM_LPC_GPPWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <xcore/entity.h>
#include <stdint.h>
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
#endif /* HALM_PLATFORM_LPC_GPPWM_BASE_H_ */
