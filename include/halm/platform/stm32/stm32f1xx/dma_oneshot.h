/*
 * halm/platform/stm32/stm32f1xx/dma_oneshot.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM32_STM32F1XX_DMA_ONESHOT_H_
#define HALM_PLATFORM_STM32_STM32F1XX_DMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/stm32f1xx/dma_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaOneShot;

struct DmaOneShotConfig
{
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: stream number. */
  uint8_t stream;
  /** Optional: stream priority. */
  uint8_t priority;
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

  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_DMA_ONESHOT_H_ */
