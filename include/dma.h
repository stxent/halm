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
  DMA_WIDTH_WORD
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_HEADER

  enum result (*start)(void *, void *, const void *, uint32_t);
  enum result (*startList)(void *, const void *);
  void (*stop)(void *);
  void (*halt)(void *);
  void (*linkItem)(void *, void *, void *, void *, const void *, uint16_t);
  /* TODO Get the number of completed transfers */
  /* uint16_t getCount(const void *); */
};
/*----------------------------------------------------------------------------*/
struct Dma
{
  struct Entity parent;

  uint8_t channel; /* Channel may have different meaning */
  bool active; /* Transfer active flag */
};
/*----------------------------------------------------------------------------*/
/**
 * Start DMA transfer.
 * @param controller Pointer to Dma object.
 * @param dest Destination memory address.
 * @param src Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
static inline enum result dmaStart(void *controller, void *dest,
    const void *src, uint32_t size)
{
  return ((struct DmaClass *)CLASS(controller))->start(controller,
      dest, src, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Start scatter-gather DMA transfer.
 * @param controller Pointer to Dma object.
 * @param first Pointer to the first descriptor in linked list.
 * @return @b E_OK on success.
 */
static inline enum result dmaStartList(void *controller, const void *first)
{
  return ((struct DmaClass *)CLASS(controller))->startList(controller, first);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable a channel and lose data in the FIFO.
 * @param controller Pointer to Dma object.
 */
static inline void dmaStop(void *controller)
{
  ((struct DmaClass *)CLASS(controller))->stop(controller);
}
/*----------------------------------------------------------------------------*/
/**
 * Disable a channel without losing data in the FIFO.
 * @param controller Pointer to Dma object.
 */
static inline void dmaHalt(void *controller)
{
  ((struct DmaClass *)CLASS(controller))->halt(controller);
}
/*----------------------------------------------------------------------------*/
/**
 * Link next element to the linked list for scatter-gather transfers.
 * @param controller Pointer to Dma object.
 * @param current Pointer to current linked list item.
 * @param next Pointer to next linked list item or zero on list end.
 * @param dest Destination memory address.
 * @param src Source memory address.
 * @param size Size of the transfer.
 * @return @b E_OK on success.
 */
static inline void dmaLinkItem(void *controller, void *current, void *next,
    void *dest, const void *src, uint16_t size)
{
  ((struct DmaClass *)CLASS(controller))->linkItem(controller, current, next,
      dest, src, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Check whether the channel is enabled or not.
 * @param controller Pointer to Dma object.
 * @return @b true when the transmission is active or @b false otherwise.
 */
static inline bool dmaIsActive(const struct Dma *controller)
{
  return controller->active;
}
/*----------------------------------------------------------------------------*/
#endif /* DMA_H_ */
