/*
 * halm/platform/nxp/gen_1/dac_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_DAC_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_DAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <xcore/interface.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DacBase;
/*----------------------------------------------------------------------------*/
struct DacBaseConfig
{
  /** Mandatory: analog output. */
  pinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct DacBase
{
  struct Interface base;

  void *reg;

  /* Output pin */
  pinNumber pin;
};
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(const struct DacBase *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_DAC_BASE_H_ */
