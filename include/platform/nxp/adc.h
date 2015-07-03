/*
 * platform/nxp/adc.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_ADC_H_
#define PLATFORM_NXP_ADC_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_ADC/adc_unit.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Adc;
/*----------------------------------------------------------------------------*/
struct AdcConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Mandatory: analog input. */
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct Adc
{
  struct Interface parent;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  /* Pin descriptor */
  struct AdcPin pin;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_ADC_H_ */
