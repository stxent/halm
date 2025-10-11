/*
 * halm/platform/lpc/lpc13uxx/system.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_H_
#define HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/bits.h>
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
/* Power-down configuration register */
enum [[gnu::packed]] SysBlockPower
{
  PWR_IRCOUT  = 0,
  PWR_IRC     = 1,
  PWR_FLASH   = 2,
  PWR_BOD     = 3,
  PWR_ADC     = 4,
  PWR_SYSOSC  = 5,
  PWR_WDTOSC  = 6,
  PWR_SYSPLL  = 7,
  PWR_USBPLL  = 8,
  PWR_USBPAD  = 10
};

/* System AHB clock control register */
enum [[gnu::packed]] SysClockBranch
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
  CLK_USB         = 14,
  CLK_WWDT        = 15,
  CLK_IOCON       = 16,
  CLK_SSP1        = 18,
  CLK_PINT        = 19,
  CLK_GROUP0INT   = 23,
  CLK_GROUP1INT   = 24,
  CLK_RAM1        = 26,
  CLK_USBSRAM     = 27
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

 unsigned int sysFlashLatency(void);
 unsigned int sysFlashLatencyFromFrequency(uint32_t);
 void sysFlashLatencyUpdate(unsigned int);
 void sysFlashLatencyReset(void);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void sysClockDisable(enum SysClockBranch branch)
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1UL << branch);
}

static inline void sysClockEnable(enum SysClockBranch branch)
{
  LPC_SYSCON->SYSAHBCLKCTRL |= 1UL << branch;
}

static inline bool sysClockStatus(enum SysClockBranch branch)
{
  return (LPC_SYSCON->SYSAHBCLKCTRL & (1UL << branch)) != 0;
}

static inline void sysPowerDisable(enum SysBlockPower block)
{
  LPC_SYSCON->PDRUNCFG |= 1UL << block;
}

static inline void sysPowerEnable(enum SysBlockPower branch)
{
  LPC_SYSCON->PDRUNCFG &= ~(1UL << branch);
}

static inline bool sysPowerStatus(enum SysBlockPower block)
{
  return (LPC_SYSCON->PDRUNCFG & (1UL << block)) == 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_H_ */
