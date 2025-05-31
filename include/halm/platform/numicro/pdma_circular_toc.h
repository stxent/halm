/*
 * halm/platform/numicro/pdma_circular_toc.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_TOC_H_
#define HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_TOC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/pdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const PdmaCircularTOC;

struct PdmaCircularTOCConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: timeout value in HCLK ticks, range is 2^8 to 2^31 - 1 ticks. */
  uint32_t timeout;
  /** Mandatory: request connection. */
  enum PdmaEvent event;
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: stop after each pass. */
  bool oneshot;
  /** Optional: call a user function only in the end of the list. */
  bool silent;
};

struct PdmaCircularTOC
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

  /* Timeout counter period */
  uint16_t timeout;
  /* Timeout counter prescaler */
  uint8_t prescaler;

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
#endif /* HALM_PLATFORM_NUMICRO_PDMA_CIRCULAR_TOC_H_ */
