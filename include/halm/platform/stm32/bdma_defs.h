/*
 * halm/platform/stm32/bdma_defs.h
 * Copyright (C) 2018, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BDMA_DEFS_H_
#define HALM_PLATFORM_STM32_BDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define DMA_MAX_TRANSFER                ((1 << 16) - 1)
/*------------------Interrupt Status Register---------------------------------*/
#define ISR_GIF_GENERIC                 BIT(0)
#define ISR_TCIF_GENERIC                BIT(1)
#define ISR_HTIF_GENERIC                BIT(2)
#define ISR_TEIF_GENERIC                BIT(3)

#define ISR_GIF(ch)                     BIT_FIELD(ISR_GIF_GENERIC, (ch) << 2)
#define ISR_TCIF(ch)                    BIT_FIELD(ISR_TCIF_GENERIC, (ch) << 2)
#define ISR_HTIF(ch)                    BIT_FIELD(ISR_HTIF_GENERIC, (ch) << 2)
#define ISR_TEIF(ch)                    BIT_FIELD(ISR_TEIF_GENERIC, (ch) << 2)

#define ISR_CHANNEL_MASK(ch)            BIT_FIELD(MASK(4), (ch) << 2)

/* Irrelevant bits should be already masked */
#define ISR_CHANNEL_VALUE(reg, ch)      ((reg) >> ((ch) << 2))
/*------------------Interrupt Flag Clear Register-----------------------------*/
#define IFCR_GIF(ch)                    BIT_FIELD(BIT(0), (ch) << 2)
#define IFCR_TCIF(ch)                   BIT_FIELD(BIT(1), (ch) << 2)
#define IFCR_HTIF(ch)                   BIT_FIELD(BIT(2), (ch) << 2)
#define IFCR_TEIF(ch)                   BIT_FIELD(BIT(3), (ch) << 2)
/*------------------Channel Configuration Register----------------------------*/
#define CCR_EN                          BIT(0)
#define CCR_TCIE                        BIT(1)
#define CCR_HTIE                        BIT(2)
#define CCR_TEIE                        BIT(3)
#define CCR_DIR                         BIT(4)
#define CCR_CIRC                        BIT(5)
#define CCR_PINC                        BIT(6)
#define CCR_MINC                        BIT(7)

#define CCR_PSIZE(value)                BIT_FIELD((value), 8)
#define CCR_PSIZE_MASK                  BIT_FIELD(MASK(2), 8)
#define CCR_PSIZE_VALUE(reg)            FIELD_VALUE((reg), CCR_PSIZE_MASK, 8)

#define CCR_MSIZE(value)                BIT_FIELD((value), 10)
#define CCR_MSIZE_MASK                  BIT_FIELD(MASK(2), 10)
#define CCR_MSIZE_VALUE(reg)            FIELD_VALUE((reg), CCR_MSIZE_MASK, 10)

#define CCR_PL(value)                   BIT_FIELD((value), 12)
#define CCR_PL_MASK                     BIT_FIELD(MASK(2), 12)
#define CCR_PL_VALUE(reg)               FIELD_VALUE((reg), CCR_PL_MASK, 12)

#define CCR_MEM2MEM                     BIT(14)

#define CCR_IE_MASK                     (CCR_TCIE | CCR_HTIE | CCR_TEIE)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_BDMA_DEFS_H_ */
