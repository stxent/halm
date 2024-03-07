/*
 * halm/platform/stm32/stm32f1xx/system.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for STM32F1xx chips.
 */

#ifndef HALM_PLATFORM_STM32_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_SYSTEM_H_
#define HALM_PLATFORM_STM32_STM32F1XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] SysBlockReset
{
  RST_OTGFS   = 12,
  RST_ETHMAC  = 14,

  RST_TIM2    = 0x20 + 0,
  RST_TIM3    = 0x20 + 1,
  RST_TIM4    = 0x20 + 2,
  RST_TIM5    = 0x20 + 3,
  RST_TIM6    = 0x20 + 4,
  RST_TIM7    = 0x20 + 5,
  RST_TIM12   = 0x20 + 6,
  RST_TIM13   = 0x20 + 7,
  RST_TIM14   = 0x20 + 8,
  RST_WWDG    = 0x20 + 11,
  RST_SPI2    = 0x20 + 14,
  RST_SPI3    = 0x20 + 15,
  RST_USART2  = 0x20 + 17,
  RST_USART3  = 0x20 + 18,
  RST_UART4   = 0x20 + 19,
  RST_UART5   = 0x20 + 20,
  RST_I2C1    = 0x20 + 21,
  RST_I2C2    = 0x20 + 22,
  RST_USB     = 0x20 + 23,
  RST_CAN1    = 0x20 + 25,
  RST_CAN2    = 0x20 + 26,
  RST_BKP     = 0x20 + 27,
  RST_PWR     = 0x20 + 28,
  RST_DAC     = 0x20 + 29,

  RST_AFIO    = 0x40 + 0,
  RST_GPIOA   = 0x40 + 2,
  RST_GPIOB   = 0x40 + 3,
  RST_GPIOC   = 0x40 + 4,
  RST_GPIOD   = 0x40 + 5,
  RST_GPIOE   = 0x40 + 6,
  RST_GPIOF   = 0x40 + 7,
  RST_GPIOG   = 0x40 + 8,
  RST_ADC1    = 0x40 + 9,
  RST_ADC2    = 0x40 + 10,
  RST_TIM1    = 0x40 + 11,
  RST_SPI1    = 0x40 + 12,
  RST_TIM8    = 0x40 + 13,
  RST_USART1  = 0x40 + 14,
  RST_ADC3    = 0x40 + 15,
  RST_TIM9    = 0x40 + 19,
  RST_TIM10   = 0x40 + 20,
  RST_TIM11   = 0x40 + 21
};

enum [[gnu::packed]] SysClockBranch
{
  CLK_DMA1      = 0,
  CLK_DMA2      = 1,
  CLK_SRAM      = 2,
  CLK_FLITF     = 4,
  CLK_CRC       = 6,
  CLK_FSMC      = 8,
  CLK_SDIO      = 10,
  CLK_OTGFS     = 12,
  CLK_ETHMAC    = 14,
  CLK_ETHMACTX  = 15,
  CLK_ETHMACRX  = 16,

  CLK_TIM2      = 0x20 + 0,
  CLK_TIM3      = 0x20 + 1,
  CLK_TIM4      = 0x20 + 2,
  CLK_TIM5      = 0x20 + 3,
  CLK_TIM6      = 0x20 + 4,
  CLK_TIM7      = 0x20 + 5,
  CLK_TIM12     = 0x20 + 6,
  CLK_TIM13     = 0x20 + 7,
  CLK_TIM14     = 0x20 + 8,
  CLK_WWDG      = 0x20 + 11,
  CLK_SPI2      = 0x20 + 14,
  CLK_SPI3      = 0x20 + 15,
  CLK_USART2    = 0x20 + 17,
  CLK_USART3    = 0x20 + 18,
  CLK_UART4     = 0x20 + 19,
  CLK_UART5     = 0x20 + 20,
  CLK_I2C1      = 0x20 + 21,
  CLK_I2C2      = 0x20 + 22,
  CLK_USB       = 0x20 + 23,
  CLK_CAN1      = 0x20 + 25,
  CLK_CAN2      = 0x20 + 26,
  CLK_BKP       = 0x20 + 27,
  CLK_PWR       = 0x20 + 28,
  CLK_DAC       = 0x20 + 29,

  CLK_AFIO      = 0x40 + 0,
  CLK_GPIOA     = 0x40 + 2,
  CLK_GPIOB     = 0x40 + 3,
  CLK_GPIOC     = 0x40 + 4,
  CLK_GPIOD     = 0x40 + 5,
  CLK_GPIOE     = 0x40 + 6,
  CLK_GPIOF     = 0x40 + 7,
  CLK_GPIOG     = 0x40 + 8,
  CLK_ADC1      = 0x40 + 9,
  CLK_ADC2      = 0x40 + 10,
  CLK_TIM1      = 0x40 + 11,
  CLK_SPI1      = 0x40 + 12,
  CLK_TIM8      = 0x40 + 13,
  CLK_USART1    = 0x40 + 14,
  CLK_ADC3      = 0x40 + 15,
  CLK_TIM9      = 0x40 + 19,
  CLK_TIM10     = 0x40 + 20,
  CLK_TIM11     = 0x40 + 21
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

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_SYSTEM_H_ */
