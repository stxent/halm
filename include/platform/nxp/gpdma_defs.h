/*
 * platform/nxp/gpdma_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPDMA_DEFS_H_
#define PLATFORM_NXP_GPDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------DMA configuration register--------------------------------*/
#define DMA_ENABLE                      BIT(0)
#define DMA_BIG_ENDIAN                  BIT(1)
#define DMA_LITTLE_ENDIAN               0
/*------------------DMA Channel Control register------------------------------*/
#define CONTROL_SIZE(size)              BIT_FIELD((size), 0)
#define CONTROL_SIZE_MASK               BIT_FIELD(MASK(12), 0)
#define CONTROL_SIZE_VALUE(reg)         FIELD_VALUE((reg), CONTROL_SIZE_MASK, 0)
#define CONTROL_SRC_BURST(burst)        BIT_FIELD((burst), 12)
#define CONTROL_DST_BURST(burst)        BIT_FIELD((burst), 15)
#define CONTROL_SRC_WIDTH(width)        BIT_FIELD((width), 18)
#define CONTROL_DST_WIDTH(width)        BIT_FIELD((width), 21)
#define CONTROL_SRC_MASTER(channel)     BIT_FIELD((channel), 24)
#define CONTROL_DST_MASTER(channel)     BIT_FIELD((channel), 25)
#define CONTROL_SRC_INC                 BIT(26) /* Source increment */
#define CONTROL_DST_INC                 BIT(27) /* Destination increment */
#define CONTROL_INT                     BIT(31) /* Terminal count interrupt */
/*------------------DMA Channel Configuration register------------------------*/
#define CONFIG_ENABLE                   BIT(0)
#define CONFIG_SRC_PERIPH(periph)       BIT_FIELD((periph), 1)
#define CONFIG_DST_PERIPH(periph)       BIT_FIELD((periph), 6)
/* Transfer type */
#define CONFIG_TYPE(type)               BIT_FIELD((type), 11)
/* Interrupt error mask */
#define CONFIG_IE                       BIT(14)
/* Terminal count interrupt mask */
#define CONFIG_ITC                      BIT(15)
/* Enable locked transfer */
#define CONFIG_LOCK                     BIT(16)
/* Indicates whether FIFO not empty */
#define CONFIG_ACTIVE                   BIT(17)
#define CONFIG_HALT                     BIT(18)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_DMA_DEFS_H_ */
