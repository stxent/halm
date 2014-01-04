/*
 * platform/nxp/lpc17xx/gpio_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPIO_DEFS_
#define GPIO_DEFS_
/*----------------------------------------------------------------------------*/
#include <macro.h>
/*------------------Values for function and mode select registers-------------*/
#define PIN_MASK                        0x03
#define PIN_OFFSET(value, offset) \
    ((unsigned long)((value) << (((offset) & 0x0F) << 1UL)))
/*------------------Pin output mode control values----------------------------*/
#define PIN_MODE_PULLUP                 0
#define PIN_MODE_INACTIVE               2
#define PIN_MODE_PULLDOWN               3
/*------------------Overall interrupt status register-------------------------*/
#define STATUS_P0INT                    BIT(0)
#define STATUS_P2INT                    BIT(2)
/*----------------------------------------------------------------------------*/
#endif /* GPIO_DEFS_ */
