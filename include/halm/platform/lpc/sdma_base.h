/*
 * halm/platform/lpc/sdma_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_BASE_H_
#define HALM_PLATFORM_LPC_SDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/sdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
#define SDMA_MAX_TRANSFER_SIZE ((1 << 10) - 1)
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum [[gnu::packed]] SdmaBurst
{
  DMA_BURST_1,
  DMA_BURST_2,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16,
  DMA_BURST_32,
  DMA_BURST_64,
  DMA_BURST_128,
  DMA_BURST_256,
  DMA_BURST_512,
  DMA_BURST_1024
};

/** DMA channel reserved values */
enum [[gnu::packed]] SdmaChannel
{
  SDMA_CHANNEL_AUTO = 0xFF
};

/** DMA transfer stride. */
enum [[gnu::packed]] SdmaStride
{
  SDMA_STRIDE_NONE,
  SDMA_STRIDE_1,
  SDMA_STRIDE_2,
  SDMA_STRIDE_4
};

/** DMA transfer width. */
enum [[gnu::packed]] SdmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
};
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] SdmaEntry
{
  uint32_t config;
  uint32_t source;
  uint32_t destination;
  uint32_t next;
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SdmaBase;

struct SdmaBaseConfig
{
  /** Mandatory: request type. */
  enum SdmaRequest request;
  /** Mandatory: request type. */
  enum SdmaTrigger trigger;
  /** Optional: channel number. */
  uint8_t channel;
  /**
   * Optional: channel priority. Eight priority levels are supported,
   * 0 means lowest priority and 7 means highest priority.
   */
  uint8_t priority;
  /** Optional: trigger polarity. */
  bool polarity;
};

struct SdmaSettings
{
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum SdmaBurst burst;
  /** Mandatory: transfer width. */
  enum SdmaWidth width;

  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: increment size of the source address. */
    enum SdmaStride stride;
    /** Mandatory: enable burst wrapping. */
    bool wrap;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: increment size of the destination address. */
    enum SdmaStride stride;
    /** Mandatory: enable burst wrapping. */
    bool wrap;
  } destination;
};

struct SdmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum Result);

  /* Pointer to the Queue Head entry */
  struct SdmaEntry *head;
  /* Precalculated value of the channel configuration register */
  uint32_t config;
  /* Trigger multiplexer value */
  enum SdmaTrigger mux;
  /* Identifier of the channel */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void sdmaResetInstance(uint8_t);
bool sdmaSetInstance(uint8_t, struct SdmaBase *);
void sdmaSetMux(struct SdmaBase *);

uint32_t sdmaBaseCalcTransferConfig(const struct SdmaBase *,
    const struct SdmaSettings *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMA_BASE_H_ */
