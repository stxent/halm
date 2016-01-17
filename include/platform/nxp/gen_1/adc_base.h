/*
 * platform/nxp/gen_1/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_ADC_BASE_H_
#define PLATFORM_NXP_GEN_1_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <entity.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnitBase;
/*----------------------------------------------------------------------------*/
struct AdcUnitBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct AdcUnitBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
void adcConfigPin(const struct AdcUnitBase *, pinNumber, struct AdcPin *);
void adcReleasePin(struct AdcPin);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_ADC_BASE_H_ */
