/*
 * halm/platform/numicro/pdma_circular.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_H_
#define HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/pdma_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const PdmaCircular;

struct PdmaCircularConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request connection. */
  enum PdmaEvent event;
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: stop after each pass. */
  bool oneshot;
  /** Optional: call a user function only in the end of the list. */
  bool silent;
};

struct PdmaCircular
{
  struct PdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct PdmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Current size of the list */
  size_t queued;

  /* State of the transfer */
  uint8_t state;
  /* Enable fixed priority mode */
  bool fixed;
  /* Stop after each pass. */
  bool oneshot;
  /* Call a user function only in the end of the list */
  bool silent;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_H_ */
