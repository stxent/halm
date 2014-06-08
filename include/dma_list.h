/*
 * dma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Scatter-gather lists extension to the Direct Memory Access interface.
 */

#ifndef DMA_LIST_H_
#define DMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaListClass
{
  struct DmaClass parent;

  enum result (*append)(void *, void *, const void *, uint32_t);
  void (*clear)(void *);
  enum result (*execute)(void *);
};
/*----------------------------------------------------------------------------*/
struct DmaList
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
/**
 * Append new element to the buffer list.
 * @param list Pointer to a DmaList object.
 * @param destination Destination memory address.
 * @param source Source memory address.
 * @param size Transfer size in number of transfers.
 * @return @b E_OK on success.
 */
static inline enum result dmaListAppend(void *list, void *destination,
    const void *source, uint32_t size)
{
  return ((struct DmaListClass *)CLASS(list))->append(list, destination,
      source, size);
}
/*----------------------------------------------------------------------------*/
/**
 * Clear the buffer list.
 * @param list Pointer to a DmaList object to be cleared.
  */
static inline void dmaListClear(void *list)
{
  ((struct DmaListClass *)CLASS(list))->clear(list);
}
/*----------------------------------------------------------------------------*/
/**
 * Start the scatter-gather transfer.
 * @param channel Pointer to a DmaList object.
 * @return @b E_OK on success.
 */
static inline enum result dmaListExecute(void *channel)
{
  return ((struct DmaListClass *)CLASS(channel))->execute(channel);
}
/*----------------------------------------------------------------------------*/
#endif /* DMA_LIST_H_ */
