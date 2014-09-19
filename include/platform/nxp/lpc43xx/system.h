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
void sysResetEnable(enum sysDeviceReset);
void sysResetDisable(enum sysDeviceReset);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_SYSTEM_H_ */
