/*
 * halm/platform/stm32/stm32f4xx/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_ADC_BASE_H_
#define HALM_PLATFORM_STM32_STM32F4XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
/*----------------------------------------------------------------------------*/
/** Symbolic names for ADC peripherals. */
enum
{
  ADC1,
  ADC2,
  ADC3
};

/* ADC trigger sources for regular group */
enum [[gnu::packed]] AdcEvent
{
  ADC_TIM1_CC1  = 0,
  ADC_TIM1_CC2  = 1,
  ADC_TIM1_CC3  = 2,
  ADC_TIM2_CC2  = 3,
  ADC_TIM2_CC3  = 4,
  ADC_TIM2_CC4  = 5,
  ADC_TIM2_TRGO = 6,
  ADC_TIM3_CC1  = 7,
  ADC_TIM3_TRGO = 8,
  ADC_TIM4_CC4  = 9,
  ADC_TIM5_CC1  = 10,
  ADC_TIM5_CC2  = 11,
  ADC_TIM5_CC3  = 12,
  ADC_TIM8_CC1  = 13,
  ADC_TIM8_TRGO = 14,
  ADC_EXTI11    = 15,
  ADC_SOFTWARE,
  ADC_EVENT_END
};

/* ADC trigger sources for injected group */
enum [[gnu::packed]] AdcInjectedEvent
{
  ADC_INJ_TIM1_CC4  = 0,
  ADC_INJ_TIM1_TRGO = 1,
  ADC_INJ_TIM2_CC1  = 2,
  ADC_INJ_TIM2_TRGO = 3,
  ADC_INJ_TIM3_CC2  = 4,
  ADC_INJ_TIM3_CC4  = 5,
  ADC_INJ_TIM4_CC1  = 6,
  ADC_INJ_TIM4_CC2  = 7,
  ADC_INJ_TIM4_CC3  = 8,
  ADC_INJ_TIM4_TRGO = 9,
  ADC_INJ_TIM5_CC4  = 10,
  ADC_INJ_TIM5_TRGO = 11,
  ADC_INJ_TIM8_CC2  = 12,
  ADC_INJ_TIM8_CC3  = 13,
  ADC_INJ_TIM8_CC4  = 14,
  ADC_INJ_EXTI15    = 15,
  ADC_INJ_SOFTWARE,
  ADC_INJ_EVENT_END
};

/* ADC sampling time */
enum [[gnu::packed]] AdcSamplingTime
{
  ADC_SAMPLING_TIME_3 = 0,
  ADC_SAMPLING_TIME_15 = 1,
  ADC_SAMPLING_TIME_28 = 2,
  ADC_SAMPLING_TIME_56 = 3,
  ADC_SAMPLING_TIME_84 = 4,
  ADC_SAMPLING_TIME_112 = 5,
  ADC_SAMPLING_TIME_144 = 6,
  ADC_SAMPLING_TIME_480 = 7,
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
extern void adcBaseHandler1(void);
extern void adcBaseHandler2(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_ADC_BASE_H_ */
