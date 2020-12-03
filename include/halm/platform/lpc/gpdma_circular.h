/*
 * halm/platform/lpc/gpdma_circular.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPDMA_CIRCULAR_H_
#define HALM_PLATFORM_LPC_GPDMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gpdma_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDmaCircular;

struct GpDmaCircularConfig
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

struct GpDmaCircular
{
  struct GpDmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct GpDmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
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
#endif /* HALM_PLATFORM_LPC_GPDMA_CIRCULAR_H_ */
