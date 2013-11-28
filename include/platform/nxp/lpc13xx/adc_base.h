/*
 * platform/nxp/lpc13xx/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_BASE_H_
#define ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#define ADC_RESOLUTION 10
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum
{
  ADC_EVENT_UNUSED      = 0,
  ADC_EVENT_PIO0_2      = 2,
  ADC_EVENT_PIO1_5      = 3,
  ADC_EVENT_CT32B0_MAT0 = 4,
  ADC_EVENT_CT32B0_MAT1 = 5,
  ADC_EVENT_CT16B0_MAT0 = 6,
  ADC_EVENT_CT16B0_MAT1 = 7
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_BASE_H_ */
