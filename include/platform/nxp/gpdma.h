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
extern const struct DmaClass *GpDma;
/*----------------------------------------------------------------------------*/
enum gpDmaType
{
  GPDMA_TYPE_M2M = 0, /* Memory to memory */
  GPDMA_TYPE_M2P, /* Memory to peripheral */
  GPDMA_TYPE_P2M /* Peripheral to memory */
};
/*----------------------------------------------------------------------------*/
struct GpDmaConfig
{
  /* Mandatory: source of the transaction request */
  dma_event_t event;
  /* Mandatory: peripheral channel number */
  uint8_t channel;
  /* Mandatory: destination configuration */
  struct {
      bool increment;
  } destination;
  /* Mandatory: source configuration */
  struct {
      bool increment;
  } source;
  /* Mandatory: number of transfers that make up a burst transfer request */
  enum dmaBurst burst;
  /* Mandatory: transfer width */
  enum dmaWidth width;
  /* Mandatory: transfer type */
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
  uint8_t muxMask, muxValue;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_TOP_H_ */
