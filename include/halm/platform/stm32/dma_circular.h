/*
 * halm/platform/stm32/dma_circular.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_CIRCULAR_H_
#define HALM_PLATFORM_STM32_DMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaCircular;

struct DmaCircularConfig
{
  /** Mandatory: request connection to the peripheral or memory. */
  enum DmaEvent event;
  /** Optional: stream priority. */
  enum DmaPriority priority;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
  /** Optional: call a user function only in the end of the list. */
  bool silent;
};

struct DmaCircular
{
  struct DmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Base address of the memory area */
  uintptr_t memoryAddress;
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
#endif /* HALM_PLATFORM_STM32_DMA_CIRCULAR_H_ */
