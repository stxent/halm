/*
 * halm/platform/lpc/sdma_circular.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_CIRCULAR_H_
#define HALM_PLATFORM_LPC_SDMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/sdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const SdmaCircular;

struct SdmaCircularConfig
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
  /** Optional: stop after each pass. */
  bool oneshot;
  /** Optional: trigger polarity. */
  bool polarity;
  /** Optional: call a user function only in the end of the list. */
  bool silent;
};

struct SdmaCircular
{
  struct SdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct SdmaEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Current size of the list */
  size_t queued;

  /* Transfer configuration register value */
  uint32_t transferConfig;

  /* State of the transfer */
  uint8_t state;
  /* Stop after each pass. */
  bool oneshot;
  /* Call a user function only in the end of the list */
  bool silent;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMA_CIRCULAR_H_ */
