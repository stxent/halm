/*
 * platform/nxp/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_BASE_H_
#define ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
#include "platform_defs.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *AdcBase;
/*----------------------------------------------------------------------------*/
struct AdcBaseConfig
{
  gpio_t pin; /* Mandatory: analog input */
};
/*----------------------------------------------------------------------------*/
struct AdcBase
{
  struct Interface parent;

  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
//void adcSetClock(uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* ADC_BASE_H_ */
