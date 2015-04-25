/*
 * platform/nxp/gen_1/dac_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_DAC_BASE_H_
#define PLATFORM_NXP_GEN_1_DAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DacBase;
/*----------------------------------------------------------------------------*/
struct DacBaseConfig
{
  /** Mandatory: analog output. */
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct DacBase
{
  struct Interface parent;

  void *reg;

  /* Output pin */
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(const struct DacBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_DAC_BASE_H_ */
