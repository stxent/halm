/*
 * platform/nxp/lpc43xx/pin_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_PIN_DEFS_H_
#define PLATFORM_NXP_LPC43XX_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*----------------------------------------------------------------------------*/
#define SFS_FUNC_MASK                   BIT_FIELD(MASK(3), 0)
#define SFS_FUNC(func)                  BIT_FIELD((func), 0)
/*----------------------------------------------------------------------------*/
#define SFS_MODE_MASK                   BIT_FIELD(MASK(2), 3)
#define SFS_MODE_PULLUP                 BIT_FIELD(0, 3)
#define SFS_MODE_REPEATER               BIT_FIELD(1, 3)
#define SFS_MODE_INACTIVE               BIT_FIELD(2, 3)
#define SFS_MODE_PULLDOWN               BIT_FIELD(3, 3)
/*----------------------------------------------------------------------------*/
#define SFS_EHS                         BIT(5) /* Select fast slew rate */
#define SFS_EZI                         BIT(6) /* Enable input buffer */
#define SFS_ZIF                         BIT(7) /* Disable glitch filter */
/*----------------------------------------------------------------------------*/
#define SFS_STRENGTH_MASK               BIT_FIELD(MASK(2), 8)
#define SFS_STRENGTH_NORMAL             BIT_FIELD(0, 8)
#define SFS_STRENGTH_MEDIUM             BIT_FIELD(1, 8)
#define SFS_STRENGTH_HIGH               BIT_FIELD(2, 8)
#define SFS_STRENGTH_ULTRAHIGH          BIT_FIELD(3, 8)
/*----------------------------------------------------------------------------*/
#define PINTSEL_CHANNEL_MASK(channel)   BIT_FIELD(MASK(8), channel << 3)
#define PINTSEL_CHANNEL(channel, port, offset) \
    BIT_FIELD(((port) << 5) | (offset), (channel) << 3)
/*----------------------------------------------------------------------------*/
//TODO USB and I2C pins
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_PIN_DEFS_H_ */
