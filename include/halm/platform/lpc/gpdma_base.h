/*
 * halm/platform/lpc/gpdma_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPDMA_BASE_H_
#define HALM_PLATFORM_LPC_GPDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/gpdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum GpDmaBurst
{
  DMA_BURST_1,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16,
  DMA_BURST_32,
  DMA_BURST_64,
  DMA_BURST_128,
  DMA_BURST_256
} __attribute__((packed));

/** DMA transfer type. */
enum GpDmaType
{
  /** Memory to memory. */
  GPDMA_TYPE_M2M,
  /** Memory to peripheral. */
  GPDMA_TYPE_M2P,
  /** Peripheral to memory. */
  GPDMA_TYPE_P2M
} __attribute__((packed));

/** DMA transfer width. */
enum GpDmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct GpDmaEntry
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpDmaBase;

struct GpDmaBaseConfig
{
  /** Mandatory: request connection to the peripheral or memory. */
  enum GpDmaEvent event;
  /** Mandatory: transfer type. */
  enum GpDmaType type;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct GpDmaSettings
{
  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: number of transfers that make up a burst transfer request. */
    enum GpDmaBurst burst;
    /** Mandatory: source transfer width. */
    enum GpDmaWidth width;
    /** Mandatory: enable increment of the source address. */
    bool increment;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: number of transfers that make up a burst transfer request. */
    enum GpDmaBurst burst;
    /** Mandatory: destination transfer width. */
    enum GpDmaWidth width;
    /** Mandatory: enable increment of the destination address. */
    bool increment;
  } destination;
};

struct GpDmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum Result);

  /* Precalculated value of Channel Configuration register */
  uint32_t config;
  /* Precalculated values of the system connections multiplexer */
  struct GpDmaMuxConfig mux;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void gpDmaResetInstance(uint8_t);
bool gpDmaSetInstance(uint8_t, struct GpDmaBase *);
void gpDmaSetMux(struct GpDmaBase *);

uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *,
    const struct GpDmaSettings *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPDMA_BASE_H_ */
