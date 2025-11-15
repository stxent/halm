/*
 * halm/platform/lpc/gen_1/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_ADC_BASE_H_
#define HALM_PLATFORM_LPC_GEN_1_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcBase;

struct AdcBaseConfig
{
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
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

/* Common functions */
uint32_t adcSetupPins(const struct AdcBase *, struct AdcPin *,
    const PinNumber *, size_t);

/* Platform-specific functions */
struct AdcPin adcConfigPin(const struct AdcBase *, PinNumber);
struct AdcBase *adcGetInstance(uint8_t);
void adcReleasePin(struct AdcPin);
bool adcSetInstance(uint8_t, struct AdcBase *, struct AdcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_ADC_BASE_H_ */
