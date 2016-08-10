/*
 * halm/platform/nxp/gen_1/adc_unit.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_ADC_UNIT_H_
#define HALM_PLATFORM_NXP_GEN_1_ADC_UNIT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/nxp/gen_1/adc_base.h>
#include <halm/spinlock.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnit;
/*----------------------------------------------------------------------------*/
struct AdcUnitConfig
{
  /** Optional: desired clock frequency. */
  uint32_t frequency;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Optional: number of bits of accuracy. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct AdcUnit
{
  struct AdcUnitBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Protects peripheral block registers */
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
enum result adcUnitRegister(struct AdcUnit *, void (*)(void *), void *);
void adcUnitUnregister(struct AdcUnit *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_ADC_UNIT_H_ */
