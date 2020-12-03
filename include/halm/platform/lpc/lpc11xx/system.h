/*
 * halm/platform/lpc/lpc11xx/system.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_LPC11XX_SYSTEM_H_
#define HALM_PLATFORM_LPC_LPC11XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/bits.h>
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
/* Power-down configuration register */
enum SysBlockPower
{
  PWR_IRCOUT  = 0,
  PWR_IRC     = 1,
  PWR_FLASH   = 2,
  PWR_BOD     = 3,
  PWR_ADC     = 4,
  PWR_SYSOSC  = 5,
  PWR_WDTOSC  = 6,
  PWR_SYSPLL  = 7
} __attribute__((packed));

/* System AHB clock control register */
enum SysClockBranch
{
  CLK_SYS         = 0,
  CLK_ROM         = 1,
  CLK_RAM         = 2,
  CLK_FLASHREG    = 3,
  CLK_FLASHARRAY  = 4,
  CLK_I2C         = 5,
  CLK_GPIO        = 6,
  CLK_CT16B0      = 7,
  CLK_CT16B1      = 8,
  CLK_CT32B0      = 9,
  CLK_CT32B1      = 10,
  CLK_SSP0        = 11,
  CLK_UART        = 12,
  CLK_ADC         = 13,
  CLK_WDT         = 15,
  CLK_IOCON       = 16,
  CLK_CAN         = 17,
  CLK_SSP1        = 18
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

static inline void sysPowerEnable(enum SysBlockPower branch)
{
  LPC_SYSCON->PDRUNCFG &= ~(1UL << branch);
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
#endif /* HALM_PLATFORM_LPC_LPC11XX_SYSTEM_H_ */
