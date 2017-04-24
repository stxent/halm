/*
 * halm/platform/nxp/adc_burst.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_ADC_BURST_H_
#define HALM_PLATFORM_NXP_ADC_BURST_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <xcore/interface.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_unit.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcBurst;
/*----------------------------------------------------------------------------*/
struct AdcBurstConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Optional: trigger to start the conversion. */
  enum adcEvent event;
  /** Mandatory: analog input. */
  PinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct AdcBurst
{
  struct Entity base;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  void *buffer;
  /* Number of samples to be converted */
  size_t left;

  /* Pin descriptor */
  struct AdcPin pin;

  /* Hardware trigger event */
  uint8_t event;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_ADC_BURST_H_ */
