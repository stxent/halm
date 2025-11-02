/*
 * halm/platform/lpc/gen_2/adc_oneshot.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_ONESHOT_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_ONESHOT_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcOneShot;

struct AdcOneShotConfig
{
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Mandatory: analog input. */
  PinNumber pin;
  /** Mandatory: converter sequence identifier. */
  enum AdcSequence sequence;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcOneShot
{
  struct AdcBase base;

  /* Sequence control register value */
  uint32_t control;
  /* Pin descriptor */
  struct AdcPin pin;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_ONESHOT_H_ */
