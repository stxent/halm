/*
 * halm/platform/nxp/lpc17xx/system.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * System configuration functions for LPC175x and LPC176x series.
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_SYSTEM_H_
#define HALM_PLATFORM_NXP_LPC17XX_SYSTEM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
#include <xcore/bits.h>
#include <xcore/helpers.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
/* Power control for peripherals register */
enum SysBlockPower
{
  PWR_TIM0  = 1,
  PWR_TIM1  = 2,
  PWR_UART0 = 3,
  PWR_UART1 = 4,
  PWR_PWM1  = 6,
  PWR_I2C0  = 7,
  PWR_SPI   = 8,
  PWR_RTC   = 9,
  PWR_SSP1  = 10,
  PWR_ADC   = 12,
  PWR_CAN1  = 13,
  PWR_CAN2  = 14,
  PWR_GPIO  = 15,
  PWR_RIT   = 16,
  PWR_MCPWM = 17,
  PWR_QEI   = 18,
  PWR_I2C1  = 19,
  PWR_SSP0  = 21,
  PWR_TIM2  = 22,
  PWR_TIM3  = 23,
  PWR_UART2 = 24,
  PWR_UART3 = 25,
  PWR_I2C2  = 26,
  PWR_I2S   = 27,
  PWR_GPDMA = 29,
  PWR_ENET  = 30,
  PWR_USB   = 31
} __attribute__((packed));

/* Divider values for peripheral clock control registers */
enum SysClockDiv
{
  CLK_DIV1 = 0,
  CLK_DIV2 = 1,
  CLK_DIV4 = 2,
  CLK_DIV6 = 3, /* For CAN1, CAN2 and CAN Filtering */
  CLK_DIV8 = 3
};

/* Peripheral clock selection registers */
enum SysClockBranch
{
  /* PCLKSEL0 */
  CLK_WDT      = 0,
  CLK_TIMER0   = 2,
  CLK_TIMER1   = 4,
  CLK_UART0    = 6,
  CLK_UART1    = 8,
  CLK_PWM1     = 12,
  CLK_I2C0     = 14,
  CLK_SPI      = 16,
  CLK_SSP1     = 20,
  CLK_DAC      = 22,
  CLK_ADC      = 24,
  CLK_CAN1     = 26,
  CLK_CAN2     = 28,
  CLK_ACF      = 30,
  /* PCLKSEL1 */
  CLK_QEI      = 0x20 + 0,
  CLK_GPIOINT  = 0x20 + 2,
  CLK_PCB      = 0x20 + 4,
  CLK_I2C1     = 0x20 + 6,
  CLK_SSP0     = 0x20 + 10,
  CLK_TIMER2   = 0x20 + 12,
  CLK_TIMER3   = 0x20 + 14,
  CLK_UART2    = 0x20 + 16,
  CLK_UART3    = 0x20 + 18,
  CLK_I2C2     = 0x20 + 20,
  CLK_I2S      = 0x20 + 22,
  CLK_RIT      = 0x20 + 26,
  CLK_SYSCON   = 0x20 + 28,
  CLK_MC       = 0x20 + 30
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sysClockControl(enum SysClockBranch, enum SysClockDiv);
unsigned int sysFlashLatency(void);
void sysFlashLatencyUpdate(unsigned int);

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline unsigned int sysClockDivToValue(enum SysClockDiv divisor)
{
  return 1 << divisor;
}

static inline void sysPowerEnable(enum SysBlockPower block)
{
  LPC_SC->PCONP |= 1UL << block;
}

static inline void sysPowerDisable(enum SysBlockPower block)
{
  LPC_SC->PCONP &= ~(1UL << block);
}

static inline bool sysPowerStatus(enum SysBlockPower block)
{
  return (LPC_SC->PCONP & (1UL << block)) != 0;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_SYSTEM_H_ */
