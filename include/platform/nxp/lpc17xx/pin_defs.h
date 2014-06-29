/*
 * platform/nxp/lpc17xx/pin_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PIN_DEFS_H_
#define PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Values for function and mode select registers-------------*/
#define PIN_MASK                        MASK(2)
#define PIN_OFFSET(value, shift) \
    BIT_FIELD((value), (((shift) & MASK(4)) << 1))
/*------------------Pin output mode control values----------------------------*/
#define PIN_MODE_PULLUP                 0
#define PIN_MODE_REPEATER               1
#define PIN_MODE_INACTIVE               2
#define PIN_MODE_PULLDOWN               3
/*------------------Overall interrupt status register-------------------------*/
#define STATUS_P0INT                    BIT(0)
#define STATUS_P2INT                    BIT(2)
/*----------------------------------------------------------------------------*/
#endif /* PIN_DEFS_H_ */
