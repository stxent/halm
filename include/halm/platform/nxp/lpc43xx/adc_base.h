/*
 * halm/platform/nxp/lpc43xx/adc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_ADC_BASE_H_
#define HALM_PLATFORM_NXP_LPC43XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum AdcEvent
{
  ADC_BURST,
  ADC_SOFTWARE,
  ADC_CTOUT_15,
  ADC_CTOUT_8,
  ADC_TRIG0,
  ADC_TRIG1,
  ADC_MCOA2,
  ADC_EVENT_END
};
/*----------------------------------------------------------------------------*/
struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
  /* Index of the control register */
  int8_t control;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_ADC_BASE_H_ */
