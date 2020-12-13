/*
 * halm/platform/stm32/dma_base.h
 * Copyright (C) 2018, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_BASE_H_
#define HALM_PLATFORM_STM32_DMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/irq.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/dma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
/** DMA transfer priority. */
enum DmaPriority
{
  DMA_PRIORITY_LOW,
  DMA_PRIORITY_MEDIUM,
  DMA_PRIORITY_HIGH,
  DMA_PRIORITY_VERY_HIGH
} __attribute__((packed));

/** DMA transfer type. */
enum DmaType
{
  /** Peripheral to memory. */
  DMA_TYPE_P2M,
  /** Memory to peripheral. */
  DMA_TYPE_M2P,
  /** Memory to memory. */
  DMA_TYPE_M2M
} __attribute__((packed));

/** DMA transfer width. */
enum DmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DmaBase;

struct DmaBaseConfig
{
  /** Optional: stream priority. */
  enum DmaPriority priority;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
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
#endif /* HALM_PLATFORM_STM32_DMA_BASE_H_ */
