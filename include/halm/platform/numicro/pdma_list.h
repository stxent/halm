/*
 * halm/platform/numicro/pdma_list.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_LIST_H_
#define HALM_PLATFORM_NUMICRO_PDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/pdma_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const PdmaList;

struct PdmaListConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request connection. */
  enum PdmaEvent event;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct PdmaList
{
  struct PdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct PdmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Index of the last item */
  size_t index;
  /* Current size of the list */
  size_t queued;

  /* State of the transfer */
  uint8_t state;
  /* Enable fixed priority mode */
  bool fixed;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_PDMA_LIST_H_ */
