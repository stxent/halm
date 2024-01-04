/*
 * halm/platform/stm32/stm32f4xx/uart_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_UART_BASE_H_
#define HALM_PLATFORM_STM32_STM32F4XX_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
/*----------------------------------------------------------------------------*/
/** Symbolic names for USART. */
enum
{
  USART1,
  USART2,
  USART3,
  UART4,
  UART5,
  USART6
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_UART_BASE_H_ */
