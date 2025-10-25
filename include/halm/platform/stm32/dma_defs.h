/*
 * halm/platform/stm32/dma_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_DEFS_H_
#define HALM_PLATFORM_STM32_DMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma.h>
#include <xcore/bits.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define DMA_MAX_TRANSFER                ((1 << 16) - 1)
/*------------------Interrupt Status Register---------------------------------*/
#define ISR_FEIF_GENERIC                BIT(0)
#define ISR_DMEIF_GENERIC               BIT(2)
#define ISR_TEIF_GENERIC                BIT(3)
#define ISR_HTIF_GENERIC                BIT(4)
#define ISR_TCIF_GENERIC                BIT(5)
#define ISR_GENERIC_ERROR \
    (ISR_FEIF_GENERIC | ISR_DMEIF_GENERIC | ISR_TEIF_GENERIC)

#define ISR_GIF(ch)                     BIT_FIELD(ISR_GIF_GENERIC, (ch) * 6)
#define ISR_TCIF(ch)                    BIT_FIELD(ISR_TCIF_GENERIC, (ch) * 6)
#define ISR_HTIF(ch)                    BIT_FIELD(ISR_HTIF_GENERIC, (ch) * 6)
#define ISR_TEIF(ch)                    BIT_FIELD(ISR_TEIF_GENERIC, (ch) * 6)

#define ISR_CHANNEL_OFFSET(ch)          (((ch) & 1) * 6 + (((ch) & 2) ? 16 : 0))
#define ISR_CHANNEL_MASK(offset)        BIT_FIELD(MASK(6), offset)
/* Irrelevant bits should be already masked */
#define ISR_CHANNEL_VALUE(reg, offset)  ((reg) >> (offset))
/*------------------Interrupt Flag Clear Register-----------------------------*/
#define IFCR_CFEIF(ch)                  BIT_FIELD(BIT(0), (ch) * 6)
#define IFCR_CDMEIF(ch)                 BIT_FIELD(BIT(2), (ch) * 6)
#define IFCR_CTEIF(ch)                  BIT_FIELD(BIT(3), (ch) * 6)
#define IFCR_CHTIF(ch)                  BIT_FIELD(BIT(4), (ch) * 6)
#define IFCR_CTCIF(ch)                  BIT_FIELD(BIT(5), (ch) * 6)
/*------------------Stream Configuration Register-----------------------------*/
#define SCR_EN                          BIT(0)
#define SCR_DMEIE                       BIT(1)
#define SCR_TEIE                        BIT(2)
#define SCR_HTIE                        BIT(3)
#define SCR_TCIE                        BIT(4)
#define SCR_PFCTRL                      BIT(5)

#define SCR_DIR(value)                  BIT_FIELD((value), 6)
#define SCR_DIR_MASK                    BIT_FIELD(MASK(2), 6)
#define SCR_DIR_VALUE(reg)              FIELD_VALUE((reg), SCR_DIR_MASK, 6)

#define SCR_CIRC                        BIT(8)
#define SCR_PINC                        BIT(9)
#define SCR_MINC                        BIT(10)

#define SCR_PSIZE(value)                BIT_FIELD((value), 11)
#define SCR_PSIZE_MASK                  BIT_FIELD(MASK(2), 11)
#define SCR_PSIZE_VALUE(reg)            FIELD_VALUE((reg), SCR_PSIZE_MASK, 11)

#define SCR_MSIZE(value)                BIT_FIELD((value), 13)
#define SCR_MSIZE_MASK                  BIT_FIELD(MASK(2), 13)
#define SCR_MSIZE_VALUE(reg)            FIELD_VALUE((reg), SCR_MSIZE_MASK, 13)

#define SCR_PINCOS                      BIT(15)

#define SCR_PL(value)                   BIT_FIELD((value), 16)
#define SCR_PL_MASK                     BIT_FIELD(MASK(2), 16)
#define SCR_PL_VALUE(reg)               FIELD_VALUE((reg), SCR_PL_MASK, 16)

#define SCR_DBM                         BIT(18)
#define SCR_CT                          BIT(19)

#define SCR_PBURST(value)               BIT_FIELD((value), 21)
#define SCR_PBURST_MASK                 BIT_FIELD(MASK(2), 21)
#define SCR_PBURST_VALUE(reg)           FIELD_VALUE((reg), SCR_PBURST_MASK, 21)

#define SCR_MBURST(value)               BIT_FIELD((value), 23)
#define SCR_MBURST_MASK                 BIT_FIELD(MASK(2), 23)
#define SCR_MBURST_VALUE(reg)           FIELD_VALUE((reg), SCR_MBURST_MASK, 23)

#define SCR_CHSEL(value)                BIT_FIELD((value), 25)
#define SCR_CHSEL_MASK                  BIT_FIELD(MASK(2), 25)
#define SCR_CHSEL_VALUE(reg)            FIELD_VALUE((reg), SCR_CHSEL_MASK, 25)

#define SCR_IE_MASK \
    (SCR_DMEIE | SCR_TEIE | SCR_HTIE | SCR_TCIE)
/*------------------Stream FIFO Control Register------------------------------*/
#define FIFO_MAX_SIZE                   16

enum
{
  SFCR_FTH_QUARTER_FULL   = 0,
  SFCR_FTH_HALF_FULL      = 1,
  SFCR_FTH_QUARTER_EMPTY  = 2,
  SFCR_FTH_FULL           = 3
};

enum
{
  SFCR_FS_LEVEL_0 = 0,
  SFCR_FS_LEVEL_1 = 1,
  SFCR_FS_LEVEL_2 = 2,
  SFCR_FS_LEVEL_3 = 3,
  SFCR_FS_EMPTY   = 4,
  SFCR_FS_FULL    = 5
};

#define SFCR_FTH(value)                 BIT_FIELD((value), 0)
#define SFCR_FTH_MASK                   BIT_FIELD(MASK(2), 0)
#define SFCR_FTH_VALUE(reg)             FIELD_VALUE((reg), SFCR_FTH_MASK, 0)

#define SFCR_DMDIS                      BIT(2)

#define SFCR_FS(value)                  BIT_FIELD((value), 3)
#define SFCR_FS_MASK                    BIT_FIELD(MASK(3), 3)
#define SFCR_FS_VALUE(reg)              FIELD_VALUE((reg), SFCR_FS_MASK, 3)

#define SFCR_FEIF                       BIT(7)
/*----------------------------------------------------------------------------*/
static inline size_t burstToSize(enum DmaWidth width, enum DmaBurst burst)
{
  const size_t burstCount = burst ? 2 << burst : 1;
  const size_t widthBytes = 1 << width;

  return burstCount * widthBytes;
}

static inline uint32_t burstToThreshold(enum DmaWidth width,
    enum DmaBurst burst)
{
  return (burstToSize(width, burst) >> 2) - 1;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DMA_DEFS_H_ */
