/*
 * halm/platform/lpc/gpdma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPDMA_LIST_H_
#define HALM_PLATFORM_LPC_GPDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gpdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDmaList;

struct GpDmaListConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request connection to the peripheral or memory. */
  enum GpDmaEvent event;
  /** Mandatory: transfer type. */
  enum GpDmaType type;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct GpDmaList
{
  struct GpDmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct GpDmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Index of the last item */
  size_t index;
  /* Current size of the list */
  size_t queued;

  /* Control register value */
  uint32_t control;
  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPDMA_LIST_H_ */
