/*
 * halm/platform/stm32/stm32f0xx/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_ADC_BASE_H_
#define HALM_PLATFORM_STM32_STM32F0XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/bdma_base.h>
/*----------------------------------------------------------------------------*/
/** Symbolic names for ADC peripherals. */
enum
{
  ADC1
};

/* ADC trigger sources for regular group */
enum [[gnu::packed]] AdcEvent
{
  ADC_TIM1_TRGO   = 0,
  ADC_TIM1_CC4    = 1,
  ADC_TIM2_TRGO   = 2,
  ADC_TIM3_TRGO   = 3,
  ADC_TIM15_TRGO  = 4,
  ADC_SOFTWARE,
  ADC_EVENT_END
};

/* ADC trigger sources for injected group */
enum [[gnu::packed]] AdcInjectedEvent
{
  ADC_INJ_SOFTWARE,
  ADC_INJ_EVENT_END
};

/* ADC sampling time */
enum [[gnu::packed]] AdcSamplingTime
{
  ADC_SAMPLING_TIME_1P5   = 0,
  ADC_SAMPLING_TIME_7P5   = 1,
  ADC_SAMPLING_TIME_13P5  = 2,
  ADC_SAMPLING_TIME_28P5  = 3,
  ADC_SAMPLING_TIME_41P5  = 4,
  ADC_SAMPLING_TIME_55P5  = 5,
  ADC_SAMPLING_TIME_71P5  = 6,
  ADC_SAMPLING_TIME_239P5 = 7,
  ADC_SAMPLING_TIME_END
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

extern void adcBaseHandler0(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_ADC_BASE_H_ */
