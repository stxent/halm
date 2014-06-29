/*
 * platform/nxp/dac_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DAC_BASE_H_
#define DAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
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
  struct Entity parent;

  void *reg;
};
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(const struct DacBase *);
/*----------------------------------------------------------------------------*/
#endif /* DAC_BASE_H_ */
