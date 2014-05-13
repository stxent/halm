/*
 * dma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DMA_LIST_H_
#define DMA_LIST_H_
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaListClass
{
  CLASS_HEADER

  enum result (*append)(void *, void *, const void *, uint32_t);
  void (*clear)(void *);
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
#endif /* DMA_LIST_H_ */
