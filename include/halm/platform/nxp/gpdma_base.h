/*
 * halm/platform/nxp/gpdma_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPDMA_BASE_H_
#define HALM_PLATFORM_NXP_GPDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/gpdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
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
struct GpDmaEntry
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct GpDmaSettings
{
  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: number of transfers that make up a burst transfer request. */
    enum dmaBurst burst;
    /** Mandatory: source transfer width. */
    enum dmaWidth width;
    /** Mandatory: enable increment of the source address. */
    bool increment;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: number of transfers that make up a burst transfer request. */
    enum dmaBurst burst;
    /** Mandatory: destination transfer width. */
    enum dmaWidth width;
    /** Mandatory: enable increment of the destination address. */
    bool increment;
  } destination;
};
/*----------------------------------------------------------------------------*/
struct GpDmaBaseConfig
{
  /** Mandatory: request connection to the peripheral or memory. */
  enum gpDmaEvent event;
  /** Mandatory: transfer type. */
  enum gpDmaType type;
  /** Mandatory: channel number. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct GpDmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum result);

  /* Precalculated value of Channel Configuration register */
  uint32_t config;
  /* Precalculated values of the system connections multiplexer */
  struct GpDmaMuxConfig mux;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
void gpDmaClearDescriptor(uint8_t);
const struct GpDmaBase *gpDmaGetDescriptor(uint8_t);
enum result gpDmaSetDescriptor(uint8_t, struct GpDmaBase *);
void gpDmaSetMux(struct GpDmaBase *);

uint32_t gpDmaBaseCalcControl(const struct GpDmaBase *,
    const struct GpDmaSettings *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPDMA_BASE_H_ */