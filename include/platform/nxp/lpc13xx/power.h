/*
 * platform/nxp/lpc13xx/power.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef POWER_H_
#define POWER_H_
/*----------------------------------------------------------------------------*/
/* Power-down configuration register */
enum sysPowerDevice
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
  CLK_SSP0        = 11,
  CLK_UART        = 12,
  CLK_ADC         = 13,
  CLK_USBREG      = 14,
  CLK_WDT         = 15,
  CLK_IOCON       = 16,
  CLK_SSP1        = 18
};
/*----------------------------------------------------------------------------*/
void sysClockEnable(enum sysClockDevice);
void sysClockDisable(enum sysClockDevice);
void sysPowerEnable(enum sysPowerDevice);
void sysPowerDisable(enum sysPowerDevice);
/*----------------------------------------------------------------------------*/
#endif /* POWER_H_ */
