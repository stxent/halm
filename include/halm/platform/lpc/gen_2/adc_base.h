/*
 * halm/platform/lpc/gen_2/adc_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_BASE_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_BASE_H_
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
  /** Mandatory: converter sequence identifier. */
  enum AdcSequence sequence;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);

  /* Precalculated value of global control register, lower half */
  uint16_t control;

  /* Interrupt identifiers */
  struct
  {
    IrqNumber ovr;
    IrqNumber seq;
    IrqNumber thcmp;
  } irq;

  /* Sequence identifier */
  enum AdcSequence sequence;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct AdcPin adcConfigPin(const struct AdcBase *, PinNumber);
void adcEnterCalibrationMode(struct AdcBase *);
struct AdcBase *adcGetInstance(enum AdcSequence);
void adcReleasePin(struct AdcPin);
bool adcSetInstance(enum AdcSequence, struct AdcBase *, struct AdcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_BASE_H_ */
