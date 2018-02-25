/*
 * halm/target.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_TARGET_H_
#define HALM_TARGET_H_
/*----------------------------------------------------------------------------*/
#include <xcore/target.h>
/*----------------------------------------------------------------------------*/
#if defined(LPC11XX)

#  define GEN_ADC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define PLATFORM        lpc11xx
#  define PLATFORM_TYPE   nxp

#elif defined(LPC11EXX)

#  define GEN_ADC         gen_1
#  define GEN_CAN         gen_2
#  define GEN_I2C         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define PLATFORM        lpc11exx
#  define PLATFORM_TYPE   nxp

#elif defined(LPC13XX)

#  define GEN_ADC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define PLATFORM        lpc13xx
#  define PLATFORM_TYPE   nxp

#elif defined(LPC17XX)

#  define GEN_ADC         gen_1
#  define GEN_CAN         gen_1
#  define GEN_DAC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_RTC         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define GEN_USB         gen_1
#  define PLATFORM        lpc17xx
#  define PLATFORM_TYPE   nxp

#elif defined(LPC43XX)

#  define GEN_ADC         gen_1
#  define GEN_CAN         gen_2
#  define GEN_DAC         gen_1
#  define GEN_I2C         gen_1
#  define GEN_RTC         gen_1
#  define GEN_SPI         gen_1
#  define GEN_UART        gen_1
#  define PLATFORM        lpc43xx
#  define PLATFORM_TYPE   nxp

#elif defined(STM32F1XX)

#  define PLATFORM        stm32f1xx
#  define PLATFORM_TYPE   stm

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
