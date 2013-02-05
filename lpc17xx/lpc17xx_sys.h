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
/* Power Control for Peripheral registers */
enum sysPowerControl {
  PCON_TIM0     = 1,
  PCON_TIM1     = 2,
  PCON_UART0    = 3,
  PCON_UART1    = 4,
  PCON_PWM1     = 6,
  PCON_I2C0     = 7,
  PCON_SPI      = 8,
  PCON_RTC      = 9,
  PCON_SSP1     = 10,
  PCON_ADC      = 12,
  PCON_CAN1     = 13,
  PCON_CAN2     = 14,
  PCON_GPIO     = 15,
  PCON_RIT      = 16,
  PCON_MCPWM    = 17,
  PCON_QEI      = 18,
  PCON_I2C1     = 19,
  PCON_SSP0     = 21,
  PCON_TIM2     = 22,
  PCON_TIM3     = 23,
  PCON_UART2    = 24,
  PCON_UART3    = 25,
  PCON_I2C2     = 26,
  PCON_I2S      = 27,
  PCON_GPDMA    = 29,
  PCON_ENET     = 30,
  PCON_USB      = 31
};
/*----------------------------------------------------------------------------*/
/* Divider values for peripheral clock select registers */
enum sysClockDiv {
  PCLK_DIV1 = 1,
  PCLK_DIV2 = 2,
  PCLK_DIV4 = 0,
  PCLK_DIV8 = 3
};
/*------------------Offsets for PCLKSEL registers, reset values 0x00000000----*/
enum sysClockControl {
/* PCLKSEL0 register */
  PCLK_WDT      = 0,
  PCLK_TIMER0   = 2,
  PCLK_TIMER1   = 4,
  PCLK_UART0    = 6,
  PCLK_UART1    = 8,
  PCLK_PWM1     = 12,
  PCLK_I2C0     = 14,
  PCLK_SPI      = 16,
  PCLK_SSP1     = 20,
  PCLK_DAC      = 22,
  PCLK_ADC      = 24,
  PCLK_CAN1     = 26,
  PCLK_CAN2     = 28,
  PCLK_ACF      = 30,
  /* PCLKSEL1 */
  PCLK_QEI      = 0x20 + 0,
  PCLK_GPIOINT  = 0x20 + 2,
  PCLK_PCB      = 0x20 + 4,
  PCLK_I2C1     = 0x20 + 6,
  PCLK_SSP0     = 0x20 + 10,
  PCLK_TIMER2   = 0x20 + 12,
  PCLK_TIMER3   = 0x20 + 14,
  PCLK_UART2    = 0x20 + 16,
  PCLK_UART3    = 0x20 + 18,
  PCLK_I2C2     = 0x20 + 20,
  PCLK_I2S      = 0x20 + 22,
  PCLK_RIT      = 0x20 + 26,
  PCLK_SYSCON   = 0x20 + 28,
  PCLK_MC       = 0x20 + 30
};
/*----------------------------------------------------------------------------*/
//extern inline enum sysClockDiv sysGetPeriphDiv(enum sysClockControl);
extern inline void sysSetPeriphDiv(enum sysClockControl, enum sysClockDiv);
extern inline void sysPowerEnable(enum sysPowerControl);
extern inline void sysPowerDisable(enum sysPowerControl);
/*----------------------------------------------------------------------------*/
#endif /* LPC17XX_SYS_H_ */
