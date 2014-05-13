/*
 * platform/nxp/gpdma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_TOP_H_
#define GPDMA_TOP_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/gpdma.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#define GPDMA_CHANNEL_COUNT 8
#define GPDMA_MAX_TRANSFER  ((1 << 12) - 1)
/*----------------------------------------------------------------------------*/
extern const struct DmaClass *GpDma;
/*----------------------------------------------------------------------------*/
/** Direct Memory Access transfer type. */
enum gpDmaType
{
  /** Memory to memory. */
  GPDMA_TYPE_M2M = 0,
  /** Memory to peripheral. */
  GPDMA_TYPE_M2P,
  /** Peripheral to memory. */
  GPDMA_TYPE_P2M
};
/*----------------------------------------------------------------------------*/
struct GpDmaConfig
{
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Mandatory: destination configuration. */
  struct {
      bool increment;
  } destination;
  /** Mandatory: source configuration. */
  struct {
      bool increment;
  } source;
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum dmaBurst burst;
  /** Mandatory: source and destination transfer widths. */
  enum dmaWidth width;
  /** Mandatory: request connection to the peripheral or memory. */
  enum gpDmaEvent event;
  /** Mandatory: transfer type. */
  enum gpDmaType type;
};
/*----------------------------------------------------------------------------*/
struct GpDma
{
  struct Dma parent;

  void *reg;
  void (*callback)(void *);
  void *callbackArgument;

  /* Precalculated values of channel control and configuration registers */
  uint32_t control, config;
  /* Precalculated values of the connection multiplexer register */
  uint32_t muxMask, muxValue;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_TOP_H_ */
