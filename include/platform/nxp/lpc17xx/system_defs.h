/*
 * platform/nxp/lpc17xx/system_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SYSTEM_DEFS_H_
#define SYSTEM_DEFS_H_
/*------------------Flash Accelerator Configuration register------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(4), 12)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 12)
/*----------------------------------------------------------------------------*/
#endif /* SYSTEM_DEFS_H_ */
