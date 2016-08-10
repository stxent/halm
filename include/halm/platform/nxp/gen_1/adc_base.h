/*
 * halm/platform/nxp/gen_1/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <xcore/entity.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnitBase;
/*----------------------------------------------------------------------------*/
struct AdcUnitBaseConfig
{
  /** Optional: desired clock. */
  uint32_t frequency;
  /** Optional: number of bits of accuracy. */
  uint8_t accuracy;
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
#endif /* HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_ */
