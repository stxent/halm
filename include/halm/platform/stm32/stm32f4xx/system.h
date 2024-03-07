/*
 * halm/platform/stm32/stm32f4xx/system.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * System configuration functions for STM32F4xx chips.
 */

#ifndef HALM_PLATFORM_STM32_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_H_
#define HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] SysBlockReset
{
  RST_GPIOA   = 0,
  RST_GPIOB   = 1,
  RST_GPIOC   = 2,
  RST_GPIOD   = 3,
  RST_GPIOE   = 4,
  RST_GPIOF   = 5,
  RST_GPIOG   = 6,
  RST_GPIOH   = 7,
  RST_GPIOI   = 8,
  RST_CRC     = 12,
  RST_DMA1    = 21,
  RST_DMA2    = 22,
  RST_ETHMAC  = 25,
  RST_OTGHS   = 29,

  RST_DCMI    = 0x20 + 0,
  RST_CRYP    = 0x20 + 4,
  RST_HASH    = 0x20 + 5,
  RST_RNG     = 0x20 + 6,
  RST_OTGFS   = 0x20 + 7,

  RST_FSMC    = 0x40 + 0,

  RST_TIM2    = 0x60 + 0,
  RST_TIM3    = 0x60 + 1,
  RST_TIM4    = 0x60 + 2,
  RST_TIM5    = 0x60 + 3,
  RST_TIM6    = 0x60 + 4,
  RST_TIM7    = 0x60 + 5,
  RST_TIM12   = 0x60 + 6,
  RST_TIM13   = 0x60 + 7,
  RST_TIM14   = 0x60 + 8,
  RST_WWDG    = 0x60 + 11,
  RST_SPI2    = 0x60 + 14,
  RST_SPI3    = 0x60 + 15,
  RST_USART2  = 0x60 + 17,
  RST_USART3  = 0x60 + 18,
  RST_UART4   = 0x60 + 19,
  RST_UART5   = 0x60 + 20,
  RST_I2C1    = 0x60 + 21,
  RST_I2C2    = 0x60 + 22,
  RST_I2C3    = 0x60 + 23,
  RST_CAN1    = 0x60 + 25,
  RST_CAN2    = 0x60 + 26,
  RST_PWR     = 0x60 + 28,
  RST_DAC     = 0x60 + 29,

  RST_TIM1    = 0x80 + 0,
  RST_TIM8    = 0x80 + 1,
  RST_USART1  = 0x80 + 4,
  RST_USART6  = 0x80 + 5,
  RST_ADC     = 0x80 + 8,
  RST_SDIO    = 0x80 + 11,
  RST_SPI1    = 0x80 + 12,
  RST_SYSCFG  = 0x80 + 14,
  RST_TIM9    = 0x80 + 16,
  RST_TIM10   = 0x80 + 17,
  RST_TIM11   = 0x80 + 18
};

enum [[gnu::packed]] SysClockBranch
{
  CLK_GPIOA       = 0,
  CLK_GPIOB       = 1,
  CLK_GPIOC       = 2,
  CLK_GPIOD       = 3,
  CLK_GPIOE       = 4,
  CLK_GPIOF       = 5,
  CLK_GPIOG       = 6,
  CLK_GPIOH       = 7,
  CLK_GPIOI       = 8,
  CLK_CRC         = 12,
  CLK_BKPSRAM     = 18,
  CLK_CCMDATASRAM = 18,
  CLK_DMA1        = 21,
  CLK_DMA2        = 22,
  CLK_ETHMAC      = 25,
  CLK_ETHMACTX    = 26,
  CLK_ETHMACRX    = 27,
  CLK_ETHMACPTP   = 28,
  CLK_OTGHS       = 29,
  CLK_OTGHSULPI   = 30,

  CLK_DCMI        = 0x20 + 0,
  CLK_CRYP        = 0x20 + 4,
  CLK_HASH        = 0x20 + 5,
  CLK_RNG         = 0x20 + 6,
  CLK_OTGFS       = 0x20 + 7,

  CLK_FSMC        = 0x40 + 0,

  CLK_TIM2        = 0x60 + 0,
  CLK_TIM3        = 0x60 + 1,
  CLK_TIM4        = 0x60 + 2,
  CLK_TIM5        = 0x60 + 3,
  CLK_TIM6        = 0x60 + 4,
  CLK_TIM7        = 0x60 + 5,
  CLK_TIM12       = 0x60 + 6,
  CLK_TIM13       = 0x60 + 7,
  CLK_TIM14       = 0x60 + 8,
  CLK_WWDG        = 0x60 + 11,
  CLK_SPI2        = 0x60 + 14,
  CLK_SPI3        = 0x60 + 15,
  CLK_USART2      = 0x60 + 17,
  CLK_USART3      = 0x60 + 18,
  CLK_UART4       = 0x60 + 19,
  CLK_UART5       = 0x60 + 20,
  CLK_I2C1        = 0x60 + 21,
  CLK_I2C2        = 0x60 + 22,
  CLK_I2C3        = 0x60 + 23,
  CLK_CAN1        = 0x60 + 25,
  CLK_CAN2        = 0x60 + 26,
  CLK_PWR         = 0x60 + 28,
  CLK_DAC         = 0x60 + 29,

  CLK_TIM1        = 0x80 + 0,
  CLK_TIM8        = 0x80 + 1,
  CLK_USART1      = 0x80 + 4,
  CLK_USART6      = 0x80 + 5,
  CLK_ADC1        = 0x80 + 8,
  CLK_ADC2        = 0x80 + 9,
  CLK_ADC3        = 0x80 + 10,
  CLK_SDIO        = 0x80 + 11,
  CLK_SPI1        = 0x80 + 12,
  CLK_SYSCFG      = 0x80 + 14,
  CLK_TIM9        = 0x80 + 16,
  CLK_TIM10       = 0x80 + 17,
  CLK_TIM11       = 0x80 + 18
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockDisable(enum SysClockBranch);
void sysClockEnable(enum SysClockBranch);
bool sysClockStatus(enum SysClockBranch);
unsigned int sysFlashLatency(void);
void sysFlashLatencyUpdate(unsigned int);
void sysPowerScalingUpdate(bool);
void sysResetDisable(enum SysBlockReset);
void sysResetEnable(enum SysBlockReset);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_SYSTEM_H_ */
