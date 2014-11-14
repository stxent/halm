/*
 * platform/nxp/gen_1/adc_unit.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_ADC_UNIT_H_
#define PLATFORM_NXP_GEN_1_ADC_UNIT_H_
/*----------------------------------------------------------------------------*/
#include <spinlock.h>
#include <platform/nxp/gen_1/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnit;
/*----------------------------------------------------------------------------*/
struct AdcUnitConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: interrupt priority. */
  priority_t priority;
};
/*----------------------------------------------------------------------------*/
struct AdcUnit
{
  struct AdcUnitBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Access protection to a peripheral block */
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
enum result adcUnitRegister(struct AdcUnit *, void (*)(void *), void *);
void adcUnitUnregister(struct AdcUnit *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_ADC_UNIT_H_ */
