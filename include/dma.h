/*
 * dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract DMA interface for embedded systems: definitions.
 */

#ifndef DMA_H_
#define DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <entity.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
typedef uint8_t dma_event_t;
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum dmaBurst
{
  DMA_BURST_1 = 0,
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
/*----------------------------------------------------------------------------*/
/** DMA transfer width. */
enum dmaWidth
{
  DMA_WIDTH_BYTE = 0,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD,
  DMA_WIDTH_DOUBLEWORD
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_HEADER

  bool (*active)(void *);
  void (*callback)(void *, void (*)(void *), void *);
  enum result (*start)(void *, void *, const void *, uint32_t);
  void (*stop)(void *);

  /* Support for scatter-gather operations */
  void *(*listAllocate)(void *, uint32_t);
  void (*listAppend)(void *, void *, uint32_t, void *, const void *, uint32_t);
  enum result (*listStart)(void *, const void *);
};
/*----------------------------------------------------------------------------*/
struct Dma
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Check whether the channel is enabled or not.
 * @param channel Pointer to Dma object.
 * @return @b true when the transmission is active or @b false otherwise.
 */
static inline bool dmaActive(void *channel)
{
  return ((struct DmaClass *)CLASS(channel))->active(channel);
}
/*----------------------------------------------------------------------------*/
/**
 * Set callback function for the transmission completion event.
 * @param channel Pointer to Dma object.
 * @param callback Callback function.
 * @param argument Callback function argument.
 */
static inline void dmaCallback(void *channel, void (*callback)(void *),
    void *argument)
{
  ((struct DmaClass *)CLASS(channel))->callback(channel, callback, argument);
}
/*----------------------------------------------------------------------------*/
/**
 * Allocate memory for linked list used by scatter-gather transfers.
 * @param channel Pointer to Dma object.
 * @param size Size of the list in elements.
 * @return Pointer to the beginning of the allocated memory space.
 */
static inline void *dmaListAllocate(void *channel, uint32_t size)
{
  return ((struct DmaClass *)CLASS(channel))->listAllocate(channel, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Link next element to the linked list.
 * @param channel Pointer to Dma object.
 * @param list Pointer to a first element of the linked list.
 * @param index Index of the currently processing element in linked list.
 * @param destination Destination memory address.
 * @param source Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
static inline void dmaListAppend(void *channel, void *first, uint32_t index,
    void *destination, const void *source, uint32_t size)
{
  ((struct DmaClass *)CLASS(channel))->listAppend(channel, first, index,
      destination, source, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Begin the execution of previously created scatter-gather transfer.
 * @param channel Pointer to Dma object.
 * @param list Pointer to a first element of the linked list.
 * @return @b E_OK on success.
 */
static inline enum result dmaListStart(void *channel, const void *first)
{
  return ((struct DmaClass *)CLASS(channel))->listStart(channel, first);
}
/*----------------------------------------------------------------------------*/
/**
 * Start transfer.
 * @param channel Pointer to Dma object.
 * @param destination Destination memory address.
 * @param source Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
static inline enum result dmaStart(void *channel, void *destination,
    const void *source, uint32_t size)
{
  return ((struct DmaClass *)CLASS(channel))->start(channel, destination,
      source, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable a channel and lose data in the FIFO.
 * @param channel Pointer to Dma object.
 */
static inline void dmaStop(void *channel)
{
  ((struct DmaClass *)CLASS(channel))->stop(channel);
}
/*----------------------------------------------------------------------------*/
#endif /* DMA_H_ */
