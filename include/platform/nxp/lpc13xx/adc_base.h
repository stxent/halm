/*
 * platform/nxp/lpc13xx/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_BASE_H_
#define ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#define ADC_RESOLUTION 10 /* Bits */
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum adcEvent
{
  ADC_SOFTWARE = 0,
  ADC_PIN_0_2,
  ADC_PIN_1_5,
  ADC_CT32B0_MAT0,
  ADC_CT32B0_MAT1,
  ADC_CT16B0_MAT0,
  ADC_CT16B0_MAT1,
  ADC_EVENT_END
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_BASE_H_ */
