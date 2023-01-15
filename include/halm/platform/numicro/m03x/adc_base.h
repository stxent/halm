/*
 * halm/platform/numicro/m03x/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_ADC_BASE_H_
#define HALM_PLATFORM_NUMICRO_M03X_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum AdcEvent
{
  ADC_EXTERNAL  = 0,
  ADC_TIMER     = 1,
  ADC_BPWM      = 2,
  ADC_PWM       = 3,
  ADC_SOFTWARE  = 4,
  ADC_EVENT_END
} __attribute__((packed));

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_ADC_BASE_H_ */
