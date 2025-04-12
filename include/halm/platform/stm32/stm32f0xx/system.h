/*
 * halm/platform/stm32/stm32f0xx/system.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for STM32F0xx chips.
 */

#ifndef HALM_PLATFORM_STM32_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_SYSTEM_H_
#define HALM_PLATFORM_STM32_STM32F0XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] SysBlockReset
{
  RST_GPIOA   = 17,
  RST_GPIOB   = 18,
  RST_GPIOC   = 19,
  RST_GPIOD   = 20,
  RST_GPIOE   = 21,
  RST_GPIOF   = 22,
  RST_TSC     = 24,

  RST_TIM2    = 0x20 + 0,
  RST_TIM3    = 0x20 + 1,
  RST_TIM6    = 0x20 + 4,
  RST_TIM7    = 0x20 + 5,
  RST_TIM14   = 0x20 + 8,
  RST_WWDG    = 0x20 + 11,
  RST_SPI2    = 0x20 + 14,
  RST_USART2  = 0x20 + 17,
  RST_USART3  = 0x20 + 18,
  RST_USART4  = 0x20 + 19,
  RST_USART5  = 0x20 + 20,
  RST_I2C1    = 0x20 + 21,
  RST_I2C2    = 0x20 + 22,
  RST_USB     = 0x20 + 23,
  RST_CAN     = 0x20 + 25,
  RST_CRS     = 0x20 + 27,
  RST_PWR     = 0x20 + 28,
  RST_DAC     = 0x20 + 29,
  RST_CEC     = 0x20 + 30,

  RST_SYSCFG  = 0x40 + 0,
  RST_USART6  = 0x40 + 5,
  RST_USART7  = 0x40 + 6,
  RST_USART8  = 0x40 + 7,
  RST_ADC     = 0x40 + 9,
  RST_TIM1    = 0x40 + 11,
  RST_SPI1    = 0x40 + 12,
  RST_USART1  = 0x40 + 14,
  RST_TIM15   = 0x40 + 16,
  RST_TIM16   = 0x40 + 17,
  RST_TIM17   = 0x40 + 18,
  RST_DBGMCU  = 0x40 + 22
};

enum [[gnu::packed]] SysClockBranch
{
  CLK_DMA1      = 0,
  CLK_DMA2      = 1,
  CLK_SRAM      = 2,
  CLK_FLITF     = 4,
  CLK_CRC       = 6,
  CLK_GPIOA     = 17,
  CLK_GPIOB     = 18,
  CLK_GPIOC     = 19,
  CLK_GPIOD     = 20,
  CLK_GPIOE     = 21,
  CLK_GPIOF     = 22,
  CLK_TSC       = 24,

  CLK_TIM2      = 0x20 + 0,
  CLK_TIM3      = 0x20 + 1,
  CLK_TIM6      = 0x20 + 4,
  CLK_TIM7      = 0x20 + 5,
  CLK_TIM14     = 0x20 + 8,
  CLK_WWDG      = 0x20 + 11,
  CLK_SPI2      = 0x20 + 14,
  CLK_USART2    = 0x20 + 17,
  CLK_USART3    = 0x20 + 18,
  CLK_USART4    = 0x20 + 19,
  CLK_USART5    = 0x20 + 20,
  CLK_I2C1      = 0x20 + 21,
  CLK_I2C2      = 0x20 + 22,
  CLK_USB       = 0x20 + 23,
  CLK_CAN       = 0x20 + 25,
  CLK_CRS       = 0x20 + 27,
  CLK_PWR       = 0x20 + 28,
  CLK_DAC       = 0x20 + 29,
  CLK_CEC       = 0x20 + 30,

  CLK_SYSCFG    = 0x40 + 0,
  CLK_USART6    = 0x40 + 5,
  CLK_USART7    = 0x40 + 6,
  CLK_USART8    = 0x40 + 7,
  CLK_ADC       = 0x40 + 9,
  CLK_TIM1      = 0x40 + 11,
  CLK_SPI1      = 0x40 + 12,
  CLK_USART1    = 0x40 + 14,
  CLK_TIM15     = 0x40 + 16,
  CLK_TIM16     = 0x40 + 17,
  CLK_TIM17     = 0x40 + 18,
  CLK_DBGMCU    = 0x40 + 22
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);
unsigned int sysFlashLatency(void);
void sysFlashLatencyUpdate(unsigned int);
void sysResetDisable(enum SysBlockReset);
void sysResetEnable(enum SysBlockReset);
void sysResetPulse(enum SysBlockReset);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_SYSTEM_H_ */
