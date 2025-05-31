/*
 * halm/platform/numicro/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_ADC_BASE_H_
#define HALM_PLATFORM_NUMICRO_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcBase;

struct AdcBaseConfig
{
  /** Optional: resolution of the conversion results. */
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
struct AdcPin adcConfigPin(const struct AdcBase *, PinNumber);
uint32_t adcMakePinCondition(enum InputEvent);
void adcReleasePin(struct AdcPin);
uint32_t adcSetupPins(struct AdcBase *, const PinNumber *, struct AdcPin *,
    size_t);
struct Pin adcSetupTriggerPin(uint8_t, PinNumber);

/* Platform-specific functions */
struct AdcBase *adcGetInstance(uint8_t);
bool adcSetInstance(uint8_t, struct AdcBase *, struct AdcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_ADC_BASE_H_ */
