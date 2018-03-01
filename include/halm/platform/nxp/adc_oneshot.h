/*
 * halm/platform/nxp/adc_oneshot.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_ADC_ONESHOT_H_
#define HALM_PLATFORM_NXP_ADC_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
};

struct AdcOneShot
{
  struct AdcBase base;

  /* Pin descriptor */
  struct AdcPin pin;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_ADC_ONESHOT_H_ */
