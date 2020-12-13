/*
 * halm/platform/stm32/stm32f0xx/uart_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_UART_BASE_H_
#define HALM_PLATFORM_STM32_STM32F0XX_UART_BASE_H_
/*----------------------------------------------------------------------------*/
/** Symbolic names for USART. */
enum
{
  USART1,

  /* USART2 is not available on STM32F03x devices */
  USART2,

  /* USART3 and USART4 are available on STM32F07x and STM32F09x devices only */
  USART3,
  USART4,

  /* USART5 through USART8 are available on STM32F09x devices only */
  USART5,
  USART6,
  USART7,
  USART8
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_UART_BASE_H_ */
