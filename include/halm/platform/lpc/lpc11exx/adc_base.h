/*
 * halm/platform/lpc/lpc11exx/adc_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11EXX_ADC_BASE_H_
#define HALM_PLATFORM_LPC_LPC11EXX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum [[gnu::packed]] AdcEvent
{
  ADC_BURST       = 0,
  ADC_SOFTWARE    = 1,
  ADC_PIN_0_2     = 2,
  ADC_PIN_1_5     = 3,
  ADC_CT32B0_MAT0 = 4,
  ADC_CT32B0_MAT1 = 5,
  ADC_CT16B0_MAT0 = 6,
  ADC_CT16B0_MAT1 = 7,
  ADC_EVENT_END
};

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

extern void adcBaseHandler0(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11EXX_ADC_BASE_H_ */
