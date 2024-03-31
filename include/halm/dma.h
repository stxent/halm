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
/* Class descriptor */
struct DmaClass
{
  CLASS_HEADER

  void (*configure)(void *, const void *);
  void (*setCallback)(void *, void (*)(void *), void *);

  /* Transfer management */

  enum Result (*enable)(void *);
  void (*disable)(void *);
  enum Result (*residue)(const void *, size_t *);
  enum Result (*status)(const void *);

  /* List management */

  void (*append)(void *, void *, const void *, size_t);
  void (*clear)(void *);
  size_t (*queued)(const void *);
};

struct Dma
{
  struct Entity base;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

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
 * Enable the channel.
 * Channel goes into the error state when the operation fails.
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
 * Get a number of bytes awaiting the transfer.
 * @param channel Pointer to a Dma object.
 * @param count Pointer to an output variable where a number of
 * pending bytes will be written.
 * @return @b E_OK when a number of pending bytes is available and
 * other error types when the channel or the buffer aren't in correct state.
 */
static inline enum Result dmaResidue(const void *channel, size_t *count)
{
  return ((const struct DmaClass *)CLASS(channel))->residue(channel, count);
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
 * @param size Total number of bytes to transfer.
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

/**
 * Get a number of queued buffers.
 * @param channel Pointer to a Dma object.
 * @return The number of queued buffers is returned.
 */
static inline size_t dmaQueued(const void *channel)
{
  return ((const struct DmaClass *)CLASS(channel))->queued(channel);
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_DMA_H_ */
