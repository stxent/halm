/*
 * halm/platform/lpc/lpc13xx/wdt_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WDT_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_WDT_BASE_H_
#define HALM_PLATFORM_LPC_LPC13XX_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#define WDT_TIMER_RESOLUTION 24
/*----------------------------------------------------------------------------*/
enum WdtClockSource
{
  WDT_CLOCK_DEFAULT
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_WDT_BASE_H_ */
