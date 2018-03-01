/*
 * halm/platform/nxp/gen_1/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <xcore/interface.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcBase;

struct AdcBaseConfig
{
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Optional: number of clocks used for each conversion. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct AdcBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);

  /* Precalculated value of Control Register */
  uint32_t control;
  /* Unique interrupt identifier */
  IrqNumber irq;
  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void adcConfigPin(const struct AdcBase *, PinNumber, struct AdcPin *);
void adcReleasePin(struct AdcPin);
void adcResetInstance(uint8_t);
bool adcSetInstance(uint8_t, struct AdcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_ADC_BASE_H_ */
