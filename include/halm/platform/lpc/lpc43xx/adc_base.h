/*
 * halm/platform/lpc/lpc43xx/adc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_
#define HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
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
} __attribute__((packed));

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
  /* Index of the control register */
  int8_t control;
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_ */
