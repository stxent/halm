/*
 * halm/platform/lpc/lpc82x/system.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SYSTEM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_SYSTEM_H_
#define HALM_PLATFORM_LPC_LPC82X_SYSTEM_H_
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
  PWR_ACMP    = 15
};

/* Peripheral reset control register */
enum [[gnu::packed]] SysBlockReset
{
  RST_SPI0    = 0,
  RST_SPI1    = 1,
  RST_UARTFRG = 2,
  RST_UART0   = 3,
  RST_UART1   = 4,
  RST_UART2   = 5,
  RST_I2C0    = 6,
  RST_MRT     = 7,
  RST_SCT     = 8,
  RST_WKT     = 9,
  RST_GPIO    = 10,
  RST_FLASH   = 11,
  RST_ACMP    = 12,
  RST_I2C1    = 14,
  RST_I2C2    = 15,
  RST_I2C3    = 16,
  RST_ADC     = 24,
  RST_DMA     = 29
};

/* System AHB clock control register */
enum [[gnu::packed]] SysClockBranch
{
  CLK_SYS       = 0,
  CLK_ROM       = 1,
  CLK_RAM       = 2,
  CLK_FLASHREG  = 3,
  CLK_FLASH     = 4,
  CLK_I2C0      = 5,
  CLK_GPIO      = 6,
  CLK_SWM       = 7,
  CLK_SCT       = 8,
  CLK_WKT       = 9,
  CLK_MRT       = 10,
  CLK_SPI0      = 11,
  CLK_SPI1      = 12,
  CLK_CRC       = 13,
  CLK_UART0     = 14,
  CLK_UART1     = 15,
  CLK_UART2     = 16,
  CLK_WWDT      = 17,
  CLK_IOCON     = 18,
  CLK_ACMP      = 19,
  CLK_I2C1      = 21,
  CLK_I2C2      = 22,
  CLK_I2C3      = 23,
  CLK_ADC       = 24,
  CLK_MTB       = 26,
  CLK_DMA       = 29
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

unsigned int sysFlashLatency(void);
void sysFlashLatencyUpdate(unsigned int);

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

static inline void sysResetDisable(enum SysBlockReset block)
{
  LPC_SYSCON->PRESETCTRL |= 1UL << block;
}

static inline void sysResetEnable(enum SysBlockReset block)
{
  LPC_SYSCON->PRESETCTRL &= ~(1UL << block);
}

static inline void sysResetPulse(enum SysBlockReset block)
{
  LPC_SYSCON->PRESETCTRL &= ~(1UL << block);
  LPC_SYSCON->PRESETCTRL |= 1UL << block;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_SYSTEM_H_ */
