/*
 * platform/nxp/lpc11exx/system.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_
#define HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <bits.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
/* Power-down configuration register */
enum sysPowerDevice
{
  PWR_IRCOUT    = 0,
  PWR_IRC       = 1,
  PWR_FLASH     = 2,
  PWR_BOD       = 3,
  PWR_ADC       = 4,
  PWR_SYSOSC    = 5,
  PWR_WDTOSC    = 6,
  PWR_SYSPLL    = 7,
  PWR_USBPLL    = 8,  /* Only for LPC11U6x */
  PWR_USBPAD    = 10, /* Only for LPC11U6x */
  PWR_TEMPSENSE = 13  /* Only for LPC11U6x and LPC11E6x */
};
/*----------------------------------------------------------------------------*/
/* System AHB clock control register */
enum sysClockDevice
{
  CLK_SYS         = 0,
  CLK_ROM         = 1,
  CLK_RAM0        = 2,
  CLK_FLASHREG    = 3,
  CLK_FLASHARRAY  = 4,
  CLK_I2C         = 5,
  CLK_GPIO        = 6,
  CLK_CT16B0      = 7,
  CLK_CT16B1      = 8,
  CLK_CT32B0      = 9,
  CLK_CT32B1      = 10,
  CLK_SSP0        = 11,
  CLK_USART       = 12,
  CLK_ADC         = 13,
  CLK_USB         = 13, /* Only for LPC11U6x */
  CLK_WWDT        = 15,
  CLK_IOCON       = 16,
  CLK_SSP1        = 18,
  CLK_PINT        = 19,
  CLK_USART1      = 20, /* Only for LPC11U6x and LPC11E6x */
  CLK_USART2      = 21, /* Only for LPC11U6x and LPC11E6x */
  CLK_USART3_4    = 22, /* Only for LPC11U6x and LPC11E6x */
  CLK_GROUP0INT   = 23,
  CLK_GROUP1INT   = 24,
  CLK_RAM1        = 26,
  CLK_RAM2        = 27,
  CLK_CRC         = 28, /* Only for LPC11U6x and LPC11E6x */
  CLK_DMA         = 29, /* Only for LPC11U6x and LPC11E6x */
  CLK_RTC         = 30, /* Only for LPC11U6x and LPC11E6x */
  CLK_SCT0_1      = 31  /* Only for LPC11U6x and LPC11E6x */
};
/*----------------------------------------------------------------------------*/
void sysFlashLatencyUpdate(unsigned int);
unsigned int sysFlashLatency();
/*----------------------------------------------------------------------------*/
static inline void sysClockEnable(enum sysClockDevice block)
{
  LPC_SYSCON->SYSAHBCLKCTRL |= BIT(block);
}
/*----------------------------------------------------------------------------*/
static inline void sysClockDisable(enum sysClockDevice block)
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~BIT(block);
}
/*----------------------------------------------------------------------------*/
static inline void sysPowerEnable(enum sysPowerDevice block)
{
  LPC_SYSCON->PDRUNCFG &= ~BIT(block);
}
/*----------------------------------------------------------------------------*/
static inline void sysPowerDisable(enum sysPowerDevice block)
{
  LPC_SYSCON->PDRUNCFG |= BIT(block);
}
/*----------------------------------------------------------------------------*/
static inline bool sysPowerStatus(enum sysPowerDevice block)
{
  return LPC_SYSCON->PDRUNCFG & BIT(block) ? false : true;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_ */
