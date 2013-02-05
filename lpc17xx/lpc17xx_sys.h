/*
 * lpc17xx_sys.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LPC17XX_SYS_H_
#define LPC17XX_SYS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* Power control for peripherals register */
enum sysPowerDevice {
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
};
/*----------------------------------------------------------------------------*/
/* Divider values for peripheral clock control registers */
enum sysClockDiv {
  CLK_DIV1 = 1,
  CLK_DIV2 = 2,
  CLK_DIV4 = 0,
  CLK_DIV8 = 3
};
/*----------------------------------------------------------------------------*/
/* Peripheral clock selection registers */
enum sysClockDevice {
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
};
/*----------------------------------------------------------------------------*/
extern inline void sysClockControl(enum sysClockDevice, enum sysClockDiv);
extern inline void sysPowerEnable(enum sysPowerDevice);
extern inline void sysPowerDisable(enum sysPowerDevice);
/*----------------------------------------------------------------------------*/
#endif /* LPC17XX_SYS_H_ */
