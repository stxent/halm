/*
 * halm/platform/stm32/stm32f1xx/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_ADC_BASE_H_
#define HALM_PLATFORM_STM32_STM32F1XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/bdma_base.h>
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
  /* ADC1 and ADC2 triggers */
  ADC_TIM1_CC1    = 0,
  ADC_TIM1_CC2    = 1,
  ADC_TIM1_CC3    = 2,
  ADC_TIM2_CC2    = 3,
  ADC_TIM3_TRGO   = 4,
  ADC_TIM4_CC4    = 5,
  ADC_EXTI11      = 6,
  ADC_TIM8_TRGO   = 6, /* Similar as EXTI11 */

  /* ADC3 triggers */
  ADC3_TIM3_CC1   = 16 + 0,
  ADC3_TIM2_CC3   = 16 + 1,
  ADC3_TIM1_CC3   = 16 + 2,
  ADC3_TIM8_CC1   = 16 + 3,
  ADC3_TIM8_TRGO  = 16 + 4,
  ADC3_TIM5_CC1   = 16 + 5,
  ADC3_TIM5_CC3   = 16 + 6,

  ADC_SOFTWARE,
  ADC_EVENT_END
};

/* ADC trigger sources for injected group */
enum [[gnu::packed]] AdcInjectedEvent
{
  /* ADC1 and ADC2 triggers */
  ADC_INJ_TIM1_TRGO   = 0,
  ADC_INJ_TIM1_CC4    = 1,
  ADC_INJ_TIM2_TRGO   = 2,
  ADC_INJ_TIM2_CC1    = 3,
  ADC_INJ_TIM3_CC4    = 4,
  ADC_INJ_TIM4_TRGO   = 5,
  ADC_INJ_EXTI15      = 6,
  ADC_INJ_TIM8_CC4    = 6, /* Similar as EXTI15 */

  /* ADC3 triggers */
  ADC3_INJ_TIM1_TRGO  = 16 + 0,
  ADC3_INJ_TIM1_CC4   = 16 + 1,
  ADC3_INJ_TIM4_CC3   = 16 + 2,
  ADC3_INJ_TIM8_CC2   = 16 + 3,
  ADC3_INJ_TIM8_CC4   = 16 + 4,
  ADC3_INJ_TIM5_TRGO  = 16 + 5,
  ADC3_INJ_TIM5_CC4   = 16 + 6,

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
extern void adcBaseHandler1(void);
extern void adcBaseHandler2(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_ADC_BASE_H_ */
