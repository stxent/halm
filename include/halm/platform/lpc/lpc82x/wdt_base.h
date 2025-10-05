/*
 * halm/platform/lpc/lpc82x/wdt_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WDT_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_WDT_BASE_H_
#define HALM_PLATFORM_LPC_LPC82X_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#define WDT_TIMER_RESOLUTION 24
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] WdtClockSource
{
  WDT_CLOCK_DEFAULT
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_WDT_BASE_H_ */
