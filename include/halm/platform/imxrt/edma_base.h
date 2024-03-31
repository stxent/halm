/*
 * halm/platform/imxrt/edma_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_EDMA_BASE_H_
#define HALM_PLATFORM_IMXRT_EDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/edma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] EdmaPriority
{
  /** Low-priority group of channels will be used. */
  DMA_PRIORITY_LOW,
  /** High-priority group of channels will be used. */
  DMA_PRIORITY_HIGH
};

/** DMA transfer width. */
enum [[gnu::packed]] EdmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD,
  DMA_WIDTH_DOUBLEWORD
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] EdmaEntry
{
  uint32_t source;
  int16_t sourceOffset;
  uint16_t attributes;
  uint32_t numberOfBytes;
  int32_t sourceAdjustment;
  uint32_t destination;
  int16_t destinationOffset;
  uint16_t currentLoop;
  int32_t destinationAdjustment;
  uint16_t controlAndStatus;
  uint16_t beginningLoop;
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EdmaBase;

struct EdmaBaseConfig
{
  /** Mandatory: request connection. */
  enum EdmaEvent event;
  /** Mandatory: channel priority. */
  enum EdmaPriority priority;
  /** Mandatory: channel number, from 0 to 15. */
  uint8_t channel;
};

struct EdmaSettings
{
  /** Mandatory: number of transfers that make up a burst transfer request. */
  size_t burst;

  /** Mandatory: source configuration. */
  struct
  {
    /**
     * Mandatory: signed offset for the source address in bytes for
     * a single transfer. The offset should be set to zero when
     * the address increment is not used. Otherwise, the offset
     * is usually equal to the width of the single transfer.
     */
    int16_t offset;
    /** Mandatory: source transfer width. */
    enum EdmaWidth width;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /**
     * Mandatory: signed offset for the destination address in bytes for
     * a single transfer. The offset should be set to zero when
     * the address increment is not used. Otherwise, the offset
     * is usually equal to the width of the single transfer.
     */
    int16_t offset;
    /** Mandatory: destination transfer width. */
    enum EdmaWidth width;
  } destination;
};

struct EdmaBase
{
  struct Entity base;

  void *mux;
  void *reg;
  void (*handler)(void *, enum Result);

  /* DMA controller number */
  uint8_t controller;
  /* Event number */
  uint8_t event;
  /* DMA channel number in the DMA controller */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void edmaResetMux(struct EdmaBase *);
void edmaSetMux(struct EdmaBase *);
void edmaStartTransfer(struct EdmaBase *);

/* Platform-specific functions */
bool edmaBindInstance(struct EdmaBase *);
void edmaUnbindInstance(struct EdmaBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_EDMA_BASE_H_ */
