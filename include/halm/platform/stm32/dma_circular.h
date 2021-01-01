/*
 * halm/platform/stm32/dma_circular.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_CIRCULAR_H_
#define HALM_PLATFORM_STM32_DMA_CIRCULAR_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaCircular;

struct DmaCircularConfig
{
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
  /** Optional: stream priority. */
  uint8_t priority;
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

  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DMA_CIRCULAR_H_ */
