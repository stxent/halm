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

#define GEN_ADC         gen_1
#define GEN_I2C         gen_1
#define GEN_SPI         gen_1
#define GEN_UART        gen_1
#define PLATFORM        lpc11xx
#define PLATFORM_TYPE   nxp

#elif defined(LPC11EXX)

#define GEN_ADC         gen_1
#define GEN_CAN         gen_2
#define GEN_I2C         gen_1
#define GEN_SPI         gen_1
#define GEN_UART        gen_1
#define PLATFORM        lpc11exx
#define PLATFORM_TYPE   nxp

#elif defined(LPC13XX)

#define GEN_ADC         gen_1
#define GEN_I2C         gen_1
#define GEN_SPI         gen_1
#define GEN_UART        gen_1
#define GEN_USB         gen_1
#define PLATFORM        lpc13xx
#define PLATFORM_TYPE   nxp

#elif defined(LPC17XX)

#define GEN_ADC         gen_1
#define GEN_CAN         gen_1
#define GEN_DAC         gen_1
#define GEN_I2C         gen_1
#define GEN_RTC         gen_1
#define GEN_SPI         gen_1
#define GEN_UART        gen_1
#define GEN_USB         gen_1
#define PLATFORM        lpc17xx
#define PLATFORM_TYPE   nxp

#elif defined(LPC43XX)

#define GEN_ADC         gen_1
#define GEN_CAN         gen_2
#define GEN_DAC         gen_1
#define GEN_I2C         gen_1
#define GEN_RTC         gen_1
#define GEN_SPI         gen_1
#define GEN_UART        gen_1
#define PLATFORM        lpc43xx
#define PLATFORM_TYPE   nxp

#elif defined(STM32F1XX)

#define PLATFORM        stm32f1xx
#define PLATFORM_TYPE   stm

#elif defined(GENERIC)

#define PLATFORM        generic
#define PLATFORM_TYPE   linux

#else
#error "Target architecture is undefined"
#endif
/*----------------------------------------------------------------------------*/
#endif /* HALM_TARGET_H_ */
