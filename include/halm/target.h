/*
 * halm/target.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_TARGET_H_
#define HALM_TARGET_H_
/*----------------------------------------------------------------------------*/
#include <xcore/target.h>
/*----------------------------------------------------------------------------*/
#if defined(LPC11XX)

#  define GEN_ADC         gen_1
#  define GEN_BOD         gen_1
#  define GEN_FLASH       gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         gen_2
#  define GEN_PINBUS      gen_2
#  define GEN_PININT      gen_2
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define NVIC_IRQ_BITS   2
#  define PLATFORM        lpc11xx
#  define PLATFORM_TYPE   lpc

#elif defined(LPC11EXX)

#  define GEN_ADC         gen_1
#  define GEN_BOD         gen_1
#  define GEN_CAN         gen_2
#  define GEN_FLASH       gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         gen_3
#  define GEN_PINBUS      gen_3
#  define GEN_PININT      gen_3
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define NVIC_IRQ_BITS   2
#  define PLATFORM        lpc11exx
#  define PLATFORM_TYPE   lpc

#elif defined(LPC13XX)

#  define GEN_ADC         gen_1
#  define GEN_BOD         gen_1
#  define GEN_FLASH       gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         gen_2
#  define GEN_PINBUS      gen_2
#  define GEN_PININT      gen_2
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define NVIC_IRQ_BITS   3
#  define PLATFORM        lpc13xx
#  define PLATFORM_TYPE   lpc

#elif defined(LPC13UXX)

#  define GEN_ADC         gen_1
#  define GEN_BOD         gen_1
#  define GEN_I2C         gen_1
#  define GEN_FLASH       gen_1
#  define GEN_PIN         gen_3
#  define GEN_PINBUS      gen_3
#  define GEN_PININT      gen_3
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define NVIC_IRQ_BITS   3
#  define PLATFORM        lpc13uxx
#  define PLATFORM_TYPE   lpc

#elif defined(LPC17XX)

#  define GEN_ADC         gen_1
#  define GEN_BOD         lpc17xx
#  define GEN_CAN         gen_1
#  define GEN_DAC         gen_1
#  define GEN_FLASH       gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         lpc17xx
#  define GEN_PINBUS      lpc17xx
#  define GEN_PININT      lpc17xx
#  define GEN_RTC         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define NVIC_IRQ_BITS   5
#  define PLATFORM        lpc17xx
#  define PLATFORM_TYPE   lpc

#elif defined(LPC43XX) || defined(LPC43XX_M0APP) || defined(LPC43XX_M0SUB)

#  define GEN_ADC         gen_1
#  define GEN_BOD         lpc43xx
#  define GEN_CAN         gen_2
#  define GEN_DAC         gen_1
#  define GEN_FLASH       lpc43xx
#  define GEN_I2C         gen_1
#  define GEN_PIN         lpc43xx
#  define GEN_PINBUS      gen_3
#  define GEN_PININT      gen_3
#  define GEN_RTC         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define PLATFORM        lpc43xx
#  define PLATFORM_TYPE   lpc

#  ifdef LPC43XX
#    define NVIC_IRQ_BITS 3
#  else
#    define NVIC_IRQ_BITS 2
#  endif

#elif defined(M03X)

#  define GEN_TIMER       gen_1
#  define NVIC_IRQ_BITS   2
#  define PLATFORM        m03x
#  define PLATFORM_TYPE   numicro

#elif defined(M48X)

#  define GEN_TIMER       gen_2
#  define NVIC_IRQ_BITS   4
#  define PLATFORM        m48x
#  define PLATFORM_TYPE   numicro

#elif defined(STM32F0XX)

#  define GEN_ADC         gen_2
#  define GEN_I2C         gen_2
#  define GEN_PIN         gen_2
#  define GEN_UART        gen_2
#  define GEN_USB         gen_1
#  define NVIC_IRQ_BITS   2
#  define PLATFORM        stm32f0xx
#  define PLATFORM_TYPE   stm32

#elif defined(STM32F1XX)

#  define GEN_ADC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         stm32f1xx
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define NVIC_IRQ_BITS   4
#  define PLATFORM        stm32f1xx
#  define PLATFORM_TYPE   stm32

#elif defined(STM32F4XX)

#  define GEN_ADC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_PIN         gen_2
#  define GEN_UART        gen_1
#  define GEN_USB         gen_2
#  define NVIC_IRQ_BITS   4
#  define PLATFORM        stm32f4xx
#  define PLATFORM_TYPE   stm32

#else

#  define PLATFORM_TYPE   generic

#  if defined(__unix__)
#    include <unistd.h>
#    if defined(_POSIX_VERSION) || defined(_POSIX_C_SOURCE)
#      define PLATFORM    posix
#    endif
#  else
#    error "Target platform is not supported"
#  endif

#endif
/*----------------------------------------------------------------------------*/
#endif /* HALM_TARGET_H_ */
