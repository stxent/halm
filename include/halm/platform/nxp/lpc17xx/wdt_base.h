/*
 * halm/platform/nxp/lpc17xx/wdt_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_WDT_BASE_H_
#define HALM_PLATFORM_NXP_LPC17XX_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#define WDT_TIMER_RESOLUTION 32
/*----------------------------------------------------------------------------*/
enum wdtClockSource
{
  WDT_CLOCK_DEFAULT,
  WDT_CLOCK_IRC,
  WDT_CLOCK_PCLK,
  WDT_CLOCK_RTC,
  WDT_CLOCK_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_WDT_BASE_H_ */
