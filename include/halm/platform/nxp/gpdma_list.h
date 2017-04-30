/*
 * halm/platform/nxp/gpdma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GPDMA_LIST_H_
#define HALM_PLATFORM_NXP_GPDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <halm/platform/nxp/gpdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDmaList;
/*----------------------------------------------------------------------------*/
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
  /**
   * Optional: set @b true to call a user function only in the end of the list.
   */
  bool silent;
};
/*----------------------------------------------------------------------------*/
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
  /* Call user function only in the end of the list */
  bool silent;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GPDMA_LIST_H_ */
