/*
 * halm/platform/stm32/stm32f1xx/uart_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_UART_BASE_H_
#define HALM_PLATFORM_STM32_STM32F1XX_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/bdma_base.h>
/*----------------------------------------------------------------------------*/
/** Symbolic names for USART. */
enum
{
  USART1,
  USART2,
  USART3,

  /*
   * UART4 through UART5 are available on
   * STM32F105 and STM32F107 devices only.
   */
  UART4,
  UART5
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_UART_BASE_H_ */
