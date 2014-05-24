/*
 * platform/nxp/dac_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DAC_BASE_H_
#define DAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <gpio.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *DacBase;
/*----------------------------------------------------------------------------*/
struct DacBaseConfig
{
  /** Mandatory: analog output. */
  gpio_t pin;
};
/*----------------------------------------------------------------------------*/
struct DacBase
{
  struct Entity parent;

  void *reg;
};
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(struct DacBase *);
/*----------------------------------------------------------------------------*/
#endif /* DAC_BASE_H_ */
