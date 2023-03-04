/*
 * halm/platform/lpc/lpc11exx/wdt_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WDT_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11EXX_WDT_BASE_H_
#define HALM_PLATFORM_LPC_LPC11EXX_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#define WDT_TIMER_RESOLUTION 24
/*----------------------------------------------------------------------------*/
enum WdtClockSource
{
  WDT_CLOCK_DEFAULT,
  WDT_CLOCK_IRC,
  WDT_CLOCK_WDOSC,
  WDT_CLOCK_END
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11EXX_WDT_BASE_H_ */
