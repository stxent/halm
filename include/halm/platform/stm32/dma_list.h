/*
 * halm/platform/stm32/dma_list.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_LIST_H_
#define HALM_PLATFORM_STM32_DMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaList;

struct DmaListConfig
{
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: request connection to the peripheral or memory. */
  enum DmaEvent event;
  /** Optional: stream priority. */
  enum DmaPriority priority;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
};

struct DmaListEntry
{
  /* Base address of the memory area */
  uintptr_t memoryAddress;
};

struct DmaList
{
  struct DmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Linked list items */
  struct DmaListEntry *list;
  /* Maximum size of the list */
  size_t capacity;
  /* Index of the last item */
  size_t index;
  /* Current size of the list */
  size_t queued;

  /* Base address of the peripheral data register */
  uintptr_t periphAddress;
  /* Number of transfers */
  uint16_t transfers;

  /* FIFO configuration */
  uint8_t fifo;
  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DMA_LIST_H_ */
