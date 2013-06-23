/*
 * lpc13xx_sys.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LPC13XX_SYS_H_
#define LPC13XX_SYS_H_
/*----------------------------------------------------------------------------*/
/* System AHB clock control register */
enum sysClockDevice
{
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
  CLK_SSP         = 11,
  CLK_UART        = 12,
  CLK_ADC         = 13,
  CLK_USBREG      = 14,
  CLK_WDT         = 15,
  CLK_IOCON       = 16
};
/*----------------------------------------------------------------------------*/
extern inline void sysClockEnable(enum sysClockDevice);
extern inline void sysClockDisable(enum sysClockDevice);
/*----------------------------------------------------------------------------*/
#endif /* LPC13XX_SYS_H_ */
