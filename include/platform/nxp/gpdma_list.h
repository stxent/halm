/*
 * platform/nxp/gpdma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_LIST_H_
#define GPDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <dma_list.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaListClass *GpDmaList;
/*----------------------------------------------------------------------------*/
struct GpDmaListConfig
{
  /** Mandatory: parent channel. */
  struct Dma *parent;
  /** Mandatory: list size. */
  uint16_t size;
  /**
   * Optional: buffer organization selection for scatter-gather transfers.
   * Set @b true for circular buffer type or @b false for linear type.
   */
  bool circular;
};
/*----------------------------------------------------------------------------*/
struct GpDmaListItem
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct GpDmaList
{
  struct DmaList parent;

  /* Descriptor list container */
  struct GpDmaListItem *first;

  /* List capacity */
  uint16_t capacity;
  /* Current list size */
  uint16_t size;

  /* Circular buffer flag */
  bool circular;
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_LIST_H_ */
