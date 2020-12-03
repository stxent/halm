/*
 * halm/dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Direct Memory Access interface for embedded systems.
 */

#ifndef HALM_DMA_H_
#define HALM_DMA_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum DmaBurst
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

/** DMA transfer width. */
enum DmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD,
  DMA_WIDTH_DOUBLEWORD
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_HEADER

  void (*setCallback)(void *, void (*)(void *), void *);
  void (*configure)(void *, const void *);

  /* Transfer management */
  enum Result (*enable)(void *);
  void (*disable)(void *);
  size_t (*pending)(const void *);
  size_t (*residue)(const void *);
  enum Result (*status)(const void *);

  /* List management */
  void (*append)(void *, void *, const void *, size_t);
  void (*clear)(void *);
};

struct Dma
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/**
 * Set the callback function for the transmission completion event.
 * @param channel Pointer to a Dma object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void dmaSetCallback(void *channel, void (*callback)(void *),
    void *argument)
{
  ((const struct DmaClass *)CLASS(channel))->setCallback(channel, callback,
      argument);
}

/**
 * Change channel settings when the channel is already initialized.
 * @param channel Pointer to a Dma object.
 * @param config Pointer to a runtime configuration data.
 */
static inline void dmaConfigure(void *channel, const void *config)
{
  ((const struct DmaClass *)CLASS(channel))->configure(channel, config);
}

/**
 * Enable the channel.
 * @param channel Pointer to a Dma object.
 * @return @b E_OK on success.
 */
static inline enum Result dmaEnable(void *channel)
{
  return ((const struct DmaClass *)CLASS(channel))->enable(channel);
}

/**
 * Disable the channel.
 * @param channel Pointer to a Dma object.
 */
static inline void dmaDisable(void *channel)
{
  ((const struct DmaClass *)CLASS(channel))->disable(channel);
}

/**
 * Get a number of pending items.
 * @param channel Pointer to a Dma object.
 * @return The number of pending items is returned.
 */
static inline size_t dmaPending(const void *channel)
{
  return ((const struct DmaClass *)CLASS(channel))->pending(channel);
}

/**
 * Get a number of pending transfers.
 * @param channel Pointer to a Dma object.
 * @return The number of pending transfers is returned.
 */
static inline size_t dmaResidue(const void *channel)
{
  return ((const struct DmaClass *)CLASS(channel))->residue(channel);
}

/**
 * Get the status of the transfer.
 * @param channel Pointer to a Dma object.
 * @return @b E_OK when the transfer is completed successfully; @b E_BUSY when
 * the transfer is not finished yet; other error types in case of errors.
 */
static inline enum Result dmaStatus(const void *channel)
{
  return ((const struct DmaClass *)CLASS(channel))->status(channel);
}

/**
 * Append a new descriptor to the descriptor chain.
 * @param channel Pointer to a Dma object.
 * @param destination Destination memory address.
 * @param source Source memory address.
 * @param size Transfer size in number of transfers.
 */
static inline void dmaAppend(void *channel, void *destination,
    const void *source, size_t size)
{
  ((const struct DmaClass *)CLASS(channel))->append(channel, destination,
      source, size);
}

/**
 * Remove all descriptors from the descriptor chain.
 * @param channel Pointer to a Dma object.
 */
static inline void dmaClear(void *channel)
{
  ((const struct DmaClass *)CLASS(channel))->clear(channel);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_DMA_H_ */
