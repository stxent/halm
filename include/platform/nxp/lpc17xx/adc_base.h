/*
 * platform/nxp/lpc17xx/adc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_ADC_BASE_H_
#define HALM_PLATFORM_NXP_LPC17XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#define ADC_RESOLUTION 12 /* Bits */
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum adcEvent
{
  ADC_SOFTWARE,
  ADC_PIN_2_10,
  ADC_PIN_1_27,
  ADC_TIMER0_MAT1,
  ADC_TIMER0_MAT3,
  ADC_TIMER1_MAT0,
  ADC_TIMER1_MAT1,
  ADC_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_ADC_BASE_H_ */
