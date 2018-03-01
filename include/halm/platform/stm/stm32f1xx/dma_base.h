/*
 * halm/platform/stm/stm32f1xx/dma_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_DMA_BASE_H_
#define HALM_PLATFORM_STM_STM32F1XX_DMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <halm/dma.h>
#include <halm/irq.h>
/*----------------------------------------------------------------------------*/
/** Symbolic names for the DMA streams */
enum
{
  DMA1_STREAM1,
  DMA1_STREAM2,
  DMA1_STREAM3,
  DMA1_STREAM4,
  DMA1_STREAM5,
  DMA1_STREAM6,
  DMA1_STREAM7,

  DMA2_STREAM1,
  DMA2_STREAM2,
  DMA2_STREAM3,
  DMA2_STREAM4,
  DMA2_STREAM5
};

/** DMA transfer type. */
enum DmaType
{
  /** Peripheral to memory. */
  DMA_TYPE_P2M,
  /** Memory to peripheral. */
  DMA_TYPE_M2P,
  /** Memory to memory. */
  DMA_TYPE_M2M
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DmaBase;

struct DmaBaseConfig
{
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
  /** Optional: stream priority. */
  uint8_t priority;
};

struct DmaSettings
{
  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: source transfer width. */
    enum DmaWidth width;
    /** Mandatory: enable increment of the source address. */
    bool increment;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: destination transfer width. */
    enum DmaWidth width;
    /** Mandatory: enable increment of the destination address. */
    bool increment;
  } destination;
};

struct DmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum Result);

  /* Precalculated value of Channel Configuration Register */
  uint32_t config;
  /* IRQ number */
  IrqNumber irq;
  /* Identifier of the stream */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

const struct DmaBase *dmaGetInstance(uint8_t);
void dmaResetInstance(uint8_t);
bool dmaSetInstance(uint8_t, struct DmaBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_DMA_BASE_H_ */
