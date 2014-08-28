/*
 * platform/nxp/lpc43xx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_
#define PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_
/*------------------Flash Accelerator Configuration register------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(0x0F, 12)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 12)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC43XX_SYSTEM_DEFS_H_ */
