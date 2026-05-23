/*
 * halm/platform/bouffalo/dma_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_DMA_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_DMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register------------------------------------*/
enum
{
  AHBMEC_LITTLE_ENDIAN,
  AHBMEC_BIG_ENDIAN
};

#define CONFIG_SDMAEN                   BIT(0)
#define CONFIG_AHBMEC(mode)             BIT_FIELD((mode), 1)
/*------------------Channel Control register----------------------------------*/
#define CONTROL_TS(size)                BIT_FIELD((size), 0)
#define CONTROL_TS_MASK                 BIT_FIELD(MASK(12), 0)
#define CONTROL_TS_VALUE(reg)           FIELD_VALUE((reg), CONTROL_TS_MASK, 0)

#define CONTROL_SBS(burst)              BIT_FIELD((burst), 12)
#define CONTROL_SBS_MASK                BIT_FIELD(MASK(3), 12)
#define CONTROL_SBS_VALUE(reg)          FIELD_VALUE((reg), CONTROL_SBS_MASK, 12)

#define CONTROL_DBS(burst)              BIT_FIELD((burst), 15)
#define CONTROL_DBS_MASK                BIT_FIELD(MASK(3), 12)
#define CONTROL_DBS_VALUE(reg)          FIELD_VALUE((reg), CONTROL_DBS_MASK, 15)

#define CONTROL_STW(width)              BIT_FIELD((width), 18)
#define CONTROL_STW_MASK                BIT_FIELD(MASK(3), 18)
#define CONTROL_STW_VALUE(reg)          FIELD_VALUE((reg), CONTROL_STW_MASK, 18)

#define CONTROL_DTW(width)              BIT_FIELD((width), 21)
#define CONTROL_DTW_MASK                BIT_FIELD(MASK(3), 21)
#define CONTROL_DTW_VALUE(reg)          FIELD_VALUE((reg), CONTROL_DTW_MASK, 21)

#define CONTROL_IMTMMODE                BIT(24) /* Memory-to-memory mode */
#define CONTROL_SI                      BIT(26) /* Source increment */
#define CONTROL_DI                      BIT(27) /* Destination increment */
#define CONTROL_TCIEN                   BIT(31) /* Terminal count interrupt */
/*------------------Channel Configuration register----------------------------*/
#define CONFIG_CHEN                     BIT(0)

#define CONFIG_SRCPH(periph)            BIT_FIELD((periph), 1)
#define CONFIG_SRCPH_MASK               BIT_FIELD(MASK(4), 1)
#define CONFIG_SRCPH_VALUE(reg)         FIELD_VALUE((reg), CONFIG_SRCPH_MASK, 1)

#define CONFIG_DSTPH(periph)            BIT_FIELD((periph), 6)
#define CONFIG_DSTPH_MASK               BIT_FIELD(MASK(4), 6)
#define CONFIG_DSTPH_VALUE(reg)         FIELD_VALUE((reg), CONFIG_DSTPH_MASK, 6)

/* Transfer type */
#define CONFIG_FLOWCTRL(type)           BIT_FIELD((type), 11)
#define CONFIG_FLOWCTRL_MASK            BIT_FIELD(MASK(3), 11)
#define CONFIG_FLOWCTRL_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG_FLOWCTRL_MASK, 11)

/* Interrupt error mask */
#define CONFIG_IEM                      BIT(14)
/* Terminal count interrupt mask */
#define CONFIG_TCIM                     BIT(15)
/* Enable locked transfer */
#define CONFIG_LOCK                     BIT(16)
/* Indicates whether FIFO not empty */
#define CONFIG_ACTIVE                   BIT(17)
#define CONFIG_HALT                     BIT(18)

/* Linked List Item counter */
#define CONFIG_LLICOUNT_MASK            BIT_FIELD(MASK(10), 20)
#define CONFIG_LLICOUNT_VALUE(reg) \
    FIELD_VALUE((reg), CONFIG_LLICOUNT_MASK, 20)
/*------------------Synchronization register----------------------------------*/
#define SYNC_MASK                       BIT_FIELD(MASK(8), 0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_DMA_DEFS_H_ */
