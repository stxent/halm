/*
 * halm/platform/lpc/wkt_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WKT_DEFS_H_
#define HALM_PLATFORM_LPC_WKT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define TIMER_RESOLUTION                0xFFFFFFFFUL
/*------------------Control registers-----------------------------------------*/
#define CTRL_CLKSEL                     BIT(0)
#define CTRL_ALARMFLAG                  BIT(1)
#define CTRL_CLEARCTR                   BIT(2)
#define CTRL_SEL_EXTCLK                 BIT(3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_WKT_DEFS_H_ */
