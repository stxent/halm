/*
 * halm/platform/bouffalo/dma_circular.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_DMA_CIRCULAR_H_
#define HALM_PLATFORM_BOUFFALO_DMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/dma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaCircular;

struct DmaCircularConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request connection to the peripheral or memory. */
  enum DmaEvent event;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: stop after each pass. */
  bool oneshot;
  /** Optional: call a user function only in the end of the list. */
  bool silent;
};

struct DmaCircular
{
  struct DmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct DmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Current size of the list */
  size_t queued;

  /* Control register value */
  uint32_t control;

  /* State of the transfer */
  uint8_t state;
  /* Stop after each pass. */
  bool oneshot;
  /* Call a user function only in the end of the list */
  bool silent;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_DMA_CIRCULAR_H_ */
