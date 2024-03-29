/*
 * halm/platform/stm32/dma_oneshot.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_ONESHOT_H_
#define HALM_PLATFORM_STM32_DMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/dma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaOneShot;

struct DmaOneShotConfig
{
  /** Mandatory: request connection to the peripheral or memory. */
  enum DmaEvent event;
  /** Optional: stream priority. */
  enum DmaPriority priority;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
};

struct DmaOneShot
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
#endif /* HALM_PLATFORM_STM32_DMA_ONESHOT_H_ */
