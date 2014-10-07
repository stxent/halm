/*
 * platform/nxp/lpc43xx/system.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * System configuration functions for LPC43xx chips.
 */

#ifndef PLATFORM_NXP_LPC43XX_SYSTEM_H_
#define PLATFORM_NXP_LPC43XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
/* Reset control for core and peripherals register */
enum sysDeviceReset
{
  RST_CORE    = 0,
  RST_PERIPH  = 1,
  RST_MASTER  = 2,
  RST_WWDT    = 4,
  RST_CREG    = 5,
  RST_BUS     = 8,
  RST_SCU     = 9,
  RST_M0SUB   = 12,
  RST_M4      = 13,
  RST_LCD     = 16,
  RST_USB0    = 17,
  RST_USB1    = 18,
  RST_GPDMA   = 19,
  RST_SDIO    = 20,
  RST_EMC     = 21,
  RST_ENET    = 22,
  RST_FLASHA  = 25,
  RST_EEPROM  = 27,
  RST_GPIO    = 28,
  RST_FLASHB  = 29,
  RST_TIMER0  = 0x20 + 0,
  RST_TIMER1  = 0x20 + 1,
  RST_TIMER2  = 0x20 + 2,
  RST_TIMER3  = 0x20 + 3,
  RST_RIT     = 0x20 + 4,
  RST_SCT     = 0x20 + 5,
  RST_MCPWM   = 0x20 + 6,
  RST_QEI     = 0x20 + 7,
  RST_ADC0    = 0x20 + 8,
  RST_ADC1    = 0x20 + 9,
  RST_DAC     = 0x20 + 10,
  RST_USART0  = 0x20 + 12,
  RST_UART1   = 0x20 + 13,
  RST_USART2  = 0x20 + 14,
  RST_USART3  = 0x20 + 15,
  RST_I2C0    = 0x20 + 16,
  RST_I2C1    = 0x20 + 17,
  RST_SSP0    = 0x20 + 18,
  RST_SSP1    = 0x20 + 19,
  RST_I2S     = 0x20 + 20,
  RST_SPIFI   = 0x20 + 21,
  RST_CAN1    = 0x20 + 22,
  RST_CAN0    = 0x20 + 23,
  RST_M0APP   = 0x20 + 24,
  RST_SGPIO   = 0x20 + 25,
  RST_SPI     = 0x20 + 26,
  RST_ADCHS   = 0x20 + 28,
};
/*----------------------------------------------------------------------------*/
/* Enable or disable clock branches */
enum sysClockBranch
{
  /* Offsets from the beginning of the CCU1 peripheral */
  CLK_APB3_BUS      = 0x020,
  CLK_APB3_I2C1     = 0x020 + 1,
  CLK_APB3_DAC      = 0x020 + 2,
  CLK_APB3_ADC0     = 0x020 + 3,
  CLK_APB3_ADC1     = 0x020 + 4,
  CLK_APB3_CAN0     = 0x020 + 5,
  CLK_APB1_BUS      = 0x040,
  CLK_APB1_MCPWM    = 0x040 + 1,
  CLK_APB1_I2C0     = 0x040 + 2,
  CLK_APB1_I2S      = 0x040 + 3,
  CLK_APB1_CAN1     = 0x040 + 4,
  CLK_SPIFI         = 0x060,
  CLK_M4_BUS        = 0x080,
  CLK_M4_SPIFI      = 0x080 + 1,
  CLK_M4_GPIO       = 0x080 + 2,
  CLK_M4_LCD        = 0x080 + 3,
  CLK_M4_ENET       = 0x080 + 4,
  CLK_M4_USB0       = 0x080 + 5,
  CLK_M4_EMC        = 0x080 + 6,
  CLK_M4_SDIO       = 0x080 + 7,
  CLK_M4_GPDMA      = 0x080 + 8,
  CLK_M4_M4CORE     = 0x080 + 9,
  CLK_M4_SCT        = 0x080 + 13,
  CLK_M4_USB1       = 0x080 + 14,
  CLK_M4_EMCDIV     = 0x080 + 15,
  CLK_M4_FLASHA     = 0x080 + 16,
  CLK_M4_FLASHB     = 0x080 + 17,
  CLK_M4_M0APP      = 0x080 + 18,
  CLK_M4_ADCHS      = 0x080 + 19,
  CLK_M4_EEPROM     = 0x080 + 20,
  CLK_M4_WWDT       = 0x0A0,
  CLK_M4_USART0     = 0x0A0 + 1,
  CLK_M4_UART1      = 0x0A0 + 2,
  CLK_M4_SSP0       = 0x0A0 + 3,
  CLK_M4_TIMER0     = 0x0A0 + 4,
  CLK_M4_TIMER1     = 0x0A0 + 5,
  CLK_M4_SCU        = 0x0A0 + 6,
  CLK_M4_CREG       = 0x0A0 + 7,
  CLK_M4_RIT        = 0x0C0,
  CLK_M4_USART2     = 0x0C0 + 1,
  CLK_M4_USART3     = 0x0C0 + 2,
  CLK_M4_TIMER2     = 0x0C0 + 3,
  CLK_M4_TIMER3     = 0x0C0 + 4,
  CLK_M4_SSP1       = 0x0C0 + 5,
  CLK_M4_QEI        = 0x0C0 + 6,
  CLK_PERIPH_BUS    = 0x0E0,
  CLK_PERIPH_CORE   = 0x0E0 + 1,
  CLK_PERIPH_SGPIO  = 0x0E0 + 2,
  CLK_USB0          = 0x100,
  CLK_USB1          = 0x120,
  CLK_SPI           = 0x140,
  CLK_ADCHS         = 0x160,

  /* Offsets from the beginning of the CCU2 peripheral */
  CLK_AUDIO         = 0x200 + 0x020,
  CLK_APB2_USART3   = 0x200 + 0x040,
  CLK_APB2_USART2   = 0x200 + 0x060,
  CLK_APB0_UART1    = 0x200 + 0x080,
  CLK_APB0_USART0   = 0x200 + 0x0A0,
  CLK_APB2_SSP1     = 0x200 + 0x0C0,
  CLK_APB0_SSP0     = 0x200 + 0x0E0,
  CLK_SDIO          = 0x200 + 0x100
};
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum sysClockBranch);
void sysClockDisable(enum sysClockBranch);
void sysResetEnable(enum sysDeviceReset);
void sysResetDisable(enum sysDeviceReset);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_SYSTEM_H_ */
