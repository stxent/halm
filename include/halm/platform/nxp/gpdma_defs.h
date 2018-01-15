/*
 * halm/platform/nxp/gpdma_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPDMA_DEFS_H_
#define HALM_PLATFORM_NXP_GPDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define GPDMA_MAX_TRANSFER              ((1 << 12) - 1)
/*------------------Configuration register------------------------------------*/
enum
{
  DMA_LITTLE_ENDIAN,
  DMA_BIG_ENDIAN
};

#define DMA_ENABLE                      BIT(0)
#define DMA_ENDIANNESS(master, mode)    BIT_FIELD((mode), 1 + (master))
/*------------------Channel Control register----------------------------------*/
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
/*------------------Channel Configuration register----------------------------*/
#define CONFIG_ENABLE                   BIT(0)
#define CONFIG_SRC_PERIPH(periph)       BIT_FIELD((periph), 1)
#define CONFIG_DST_PERIPH(periph)       BIT_FIELD((periph), 6)
/* Transfer type */
#define CONFIG_TYPE(type)               BIT_FIELD((type), 11)
#define CONFIG_TYPE_MASK                BIT_FIELD(MASK(3), 11)
#define CONFIG_TYPE_VALUE(reg)          FIELD_VALUE((reg), CONFIG_TYPE_MASK, 11)
/* Interrupt error mask */
#define CONFIG_IE                       BIT(14)
/* Terminal count interrupt mask */
#define CONFIG_ITC                      BIT(15)
/* Enable locked transfer */
#define CONFIG_LOCK                     BIT(16)
/* Indicates whether FIFO not empty */
#define CONFIG_ACTIVE                   BIT(17)
#define CONFIG_HALT                     BIT(18)
/*------------------Synchronization register----------------------------------*/
#define SYNC_MASK                       BIT_FIELD(MASK(16), 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_DMA_DEFS_H_ */
