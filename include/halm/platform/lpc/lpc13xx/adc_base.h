/*
 * halm/platform/lpc/lpc13xx/adc_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_ADC_BASE_H_
#define HALM_PLATFORM_LPC_LPC13XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum AdcEvent
{
  ADC_BURST,
  ADC_SOFTWARE,
  ADC_PIN_0_2,
  ADC_PIN_1_5,
  ADC_CT32B0_MAT0,
  ADC_CT32B0_MAT1,
  ADC_CT16B0_MAT0,
  ADC_CT16B0_MAT1,
  ADC_EVENT_END
} __attribute__((packed));

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_ADC_BASE_H_ */
