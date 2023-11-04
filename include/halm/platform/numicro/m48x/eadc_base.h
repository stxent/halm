/*
 * halm/platform/numicro/m48x/eadc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_EADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_EADC_BASE_H_
#define HALM_PLATFORM_NUMICRO_M48X_EADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum AdcEvent
{
  ADC_SOFTWARE  = 0,
  ADC_EXTERNAL  = 1,
  ADC_ADINT0    = 2,
  ADC_ADINT1    = 3,
  ADC_TIMER0    = 4,
  ADC_TIMER1    = 5,
  ADC_TIMER2    = 6,
  ADC_TIMER3    = 7,
  ADC_EPWM0TG0  = 8,
  ADC_EPWM0TG1  = 9,
  ADC_EPWM0TG2  = 10,
  ADC_EPWM0TG3  = 11,
  ADC_EPWM0TG4  = 12,
  ADC_EPWM0TG5  = 13,
  ADC_EPWM1TG0  = 14,
  ADC_EPWM1TG1  = 15,
  ADC_EPWM1TG2  = 16,
  ADC_EPWM1TG3  = 17,
  ADC_EPWM1TG4  = 18,
  ADC_EPWM1TG5  = 19,
  ADC_BPWM0TG   = 20,
  ADC_BPWM1TG   = 21,
  ADC_EVENT_END
} __attribute__((packed));

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

extern void adcBaseHandler0(void);
extern void adcBaseHandler1(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_EADC_BASE_H_ */
