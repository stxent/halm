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
#include "lpc17xx_defs.h"
/*----------------------------------------------------------------------------*/
extern inline uint8_t sysGetPeriphDiv(enum sysPeriphClock);
extern inline void sysSetPeriphDiv(enum sysPeriphClock, uint8_t);
/*----------------------------------------------------------------------------*/
#endif /* LPC17XX_SYS_H_ */
