/*
 * platform/nxp/gpdma_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_BASE_TOP_H_
#define GPDMA_BASE_TOP_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <mcu.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/gpdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#define GPDMA_CHANNEL_COUNT 8
#define GPDMA_MAX_TRANSFER  ((1 << 12) - 1)
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const GpDmaBase;
/*----------------------------------------------------------------------------*/
/** Direct Memory Access transfer type. */
enum gpDmaType
{
  /** Memory to memory. */
  GPDMA_TYPE_M2M,
  /** Memory to peripheral. */
  GPDMA_TYPE_M2P,
  /** Peripheral to memory. */
  GPDMA_TYPE_P2M
};
/*----------------------------------------------------------------------------*/
struct GpDmaBaseConfig
{
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Mandatory: request connection to the peripheral or memory. */
  enum gpDmaEvent event;
  /** Mandatory: transfer type. */
  enum gpDmaType type;
};
/*----------------------------------------------------------------------------*/
struct GpDmaBase
{
  struct Entity parent;

  void *reg;
  void (*handler)(void *, enum result);

  /* Precalculated values of channel control and configuration registers */
  uint32_t control, config;
  /* Precalculated values of the system connections multiplexer */
  struct GpDmaMuxConfig mux;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
const struct GpDmaBase *gpDmaGetDescriptor(uint8_t);
enum result gpDmaSetDescriptor(uint8_t, struct GpDmaBase *);
void gpDmaSetupMux(struct GpDmaBase *);
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_BASE_TOP_H_ */
