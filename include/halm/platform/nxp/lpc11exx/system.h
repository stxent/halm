/*
 * halm/platform/nxp/lpc11exx/system.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_
#define HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/bits.h>
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
/* Power-down configuration register */
enum SysBlockPower
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
} __attribute__((packed));

/* System AHB clock control register */
enum SysClockBranch
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
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

unsigned int sysFlashLatency(void);
void sysFlashLatencyUpdate(unsigned int);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void sysClockEnable(enum SysClockBranch branch)
{
  LPC_SYSCON->SYSAHBCLKCTRL |= 1UL << branch;
}

static inline void sysClockDisable(enum SysClockBranch branch)
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1UL << branch);
}

static inline bool sysClockStatus(enum SysClockBranch branch)
{
  return (LPC_SYSCON->SYSAHBCLKCTRL & (1UL << branch)) != 0;
}

static inline void sysPowerEnable(enum SysBlockPower block)
{
  LPC_SYSCON->PDRUNCFG &= ~(1UL << block);
}

static inline void sysPowerDisable(enum SysBlockPower block)
{
  LPC_SYSCON->PDRUNCFG |= 1UL << block;
}

static inline bool sysPowerStatus(enum SysBlockPower block)
{
  return (LPC_SYSCON->PDRUNCFG & (1UL << block)) == 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11EXX_SYSTEM_H_ */
