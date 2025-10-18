/*
 * halm/platform/lpc/sdma_list.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_LIST_H_
#define HALM_PLATFORM_LPC_SDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/sdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const SdmaList;

struct SdmaListConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request type. */
  enum SdmaRequest request;
  /** Mandatory: request type. */
  enum SdmaTrigger trigger;
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: channel priority. */
  uint8_t priority;
  /** Optional: trigger polarity. */
  bool polarity;
};

struct SdmaList
{
  struct SdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct SdmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Index of the last item */
  size_t index;
  /* Current size of the list */
  size_t queued;

  /* Transfer configuration register value */
  uint32_t transferConfig;
  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMA_LIST_H_ */
