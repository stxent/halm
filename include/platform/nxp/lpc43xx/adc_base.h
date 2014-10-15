/*
 * platform/nxp/lpc43xx/adc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_ADC_BASE_H_
#define PLATFORM_NXP_LPC43XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#define ADC_RESOLUTION 10 /* Bits */
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum adcEvent
{
  ADC_SOFTWARE,
  ADC_CTOUT_15,
  ADC_CTOUT_8,
  ADC_TRIG0,
  ADC_TRIG1,
  ADC_MCOA2,
  ADC_EVENT_END
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_ADC_BASE_H_ */
