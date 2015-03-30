/*
 * platform/nxp/lpc11exx/adc_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC11EXX_ADC_BASE_H_
#define PLATFORM_NXP_LPC11EXX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#define ADC_RESOLUTION 10 /* Bits */
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum adcEvent
{
  ADC_SOFTWARE,
  ADC_PIN_0_2,
  ADC_PIN_1_5,
  ADC_CT32B0_MAT0,
  ADC_CT32B0_MAT1,
  ADC_CT16B0_MAT0,
  ADC_CT16B0_MAT1,
  ADC_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC11EXX_ADC_BASE_H_ */
