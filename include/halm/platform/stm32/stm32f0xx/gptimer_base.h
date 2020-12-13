/*
 * halm/platform/stm32/stm32f0xx/gptimer_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GPTIMER_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_GPTIMER_BASE_H_
#define HALM_PLATFORM_STM32_STM32F0XX_GPTIMER_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for advanced-control and general-purpose timers. */
enum
{
  TIM1  = 0,
  TIM2  = 1,
  TIM3  = 2,

  /* TIM6 is available on STM32F05x, STM32F07x and STM32F09x devices only */
  TIM6  = 5,
  /* TIM7 is available on STM32F07x and STM32F09x devices only */
  TIM7  = 6,

  TIM14 = 13,

  /* TIM15 is not available on STM32F03x devices */
  TIM15 = 14,

  TIM16 = 15,
  TIM17 = 16
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_GPTIMER_BASE_H_ */
