/*
 * lpc17xx_sys.h
 *
 *  Created on: Oct 19, 2012
 *      Author: xen
 */

#ifndef LPC17XX_SYS_H_
#define LPC17XX_SYS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#include "lpc17xx_defines.h"
/*----------------------------------------------------------------------------*/
extern inline uint8_t sysGetPeriphDiv(enum sysPeriphClock);
extern inline void sysSetPeriphDiv(enum sysPeriphClock, uint8_t);
/*----------------------------------------------------------------------------*/
#endif /* LPC17XX_SYS_H_ */
