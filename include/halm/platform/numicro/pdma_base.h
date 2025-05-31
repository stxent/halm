/*
 * halm/platform/numicro/pdma_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_BASE_H_
#define HALM_PLATFORM_NUMICRO_PDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/pdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#define PDMA_MAX_TRANSFER_SIZE ((1 << 14) - 1)
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum [[gnu::packed]] PdmaBurst
{
  DMA_BURST_1,
  DMA_BURST_2,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16,
  DMA_BURST_32,
  DMA_BURST_64,
  DMA_BURST_128
};

enum [[gnu::packed]] PdmaPriority
{
  DMA_PRIORITY_FIXED,
  DMA_PRIORITY_RR
};

/** DMA transfer width. */
enum [[gnu::packed]] PdmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] PdmaEntry
{
  uint32_t control;
  uint32_t source;
  uint32_t destination;
  uint32_t next;
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const PdmaBase;

struct PdmaBaseConfig
{
  /** Mandatory: request connection. */
  enum PdmaEvent event;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct PdmaSettings
{
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum PdmaBurst burst;
  /** Mandatory: channel priority. */
  enum PdmaPriority priority;
  /** Mandatory: transfer width. */
  enum PdmaWidth width;

  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: enable increment of the source address. */
    bool increment;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: enable increment of the destination address. */
    bool increment;
  } destination;
};

struct PdmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum Result);

  /* Precalculated value of Channel Control register */
  uint32_t control;
  /* Controller number */
  uint8_t controller;
  /* Channel number */
  uint8_t number;

  /* Event multiplexer settings */
  struct PdmaMuxConfig mux;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

bool pdmaBindInstance(struct PdmaBase *);
void pdmaUnbindInstance(struct PdmaBase *);
void pdmaSetMux(struct PdmaBase *);
void pdmaStartTransfer(struct PdmaBase *, uint32_t, uintptr_t, uintptr_t,
    uintptr_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_PDMA_BASE_H_ */
