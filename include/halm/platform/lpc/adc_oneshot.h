/*
 * halm/platform/lpc/adc_oneshot.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_ONESHOT_H_
#define HALM_PLATFORM_LPC_ADC_ONESHOT_H_
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
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcOneShot
{
  struct AdcBase base;

  /* Pin descriptor */
  struct AdcPin pin;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ADC_ONESHOT_H_ */
