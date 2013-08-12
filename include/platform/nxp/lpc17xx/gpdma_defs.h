/*
 * gpdma_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_DEFS_H_
#define GPDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include "macro.h"
/*------------------DMA configuration register--------------------------------*/
#define DMA_ENABLE                      BIT(0)
/* 0 for little-endian, 1 for big-endian */
#define DMA_ENDIANNESS                  BIT(1)
/*------------------DMA Channel Control register------------------------------*/
#define C_CONTROL_SIZE(size)            (size)
#define C_CONTROL_SIZE_MASK             0x0FFF
#define C_CONTROL_SRC_BURST(burst)      ((burst) << 12)
#define C_CONTROL_DEST_BURST(burst)     ((burst) << 15)
#define C_CONTROL_SRC_WIDTH(width)      ((width) << 18)
#define C_CONTROL_DEST_WIDTH(width)     ((width) << 21)
#define C_CONTROL_SRC_INC               BIT(26) /* Source increment */
#define C_CONTROL_DEST_INC              BIT(27) /* Destination increment */
#define C_CONTROL_INT                   BIT(31) /* Terminal count interrupt */
/*------------------DMA Channel Configuration register------------------------*/
#define C_CONFIG_ENABLE                 BIT(0)
#define C_CONFIG_SRC_PERIPH(periph)     ((periph >> 2) << 1)
#define C_CONFIG_DEST_PERIPH(periph)    ((periph >> 2) << 6)
/* Transfer type */
#define C_CONFIG_TYPE(type)             ((type) << 11)
/* Interrupt error mask */
#define C_CONFIG_IE                     BIT(14)
/* Terminal count interrupt mask */
#define C_CONFIG_ITC                    BIT(15)
/* Indicates whether FIFO not empty */
#define C_CONFIG_ACTIVE                 BIT(17)
#define C_CONFIG_HALT                   BIT(18)
/*----------------------------------------------------------------------------*/
#endif /* DMA_DEFS_H_ */
