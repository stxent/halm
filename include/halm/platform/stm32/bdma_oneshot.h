/*
 * halm/platform/stm32/gen_1/bdma_oneshot.h
 * Copyright (C) 2018, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BDMA_ONESHOT_H_
#define HALM_PLATFORM_STM32_BDMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/bdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const BdmaOneShot;

struct BdmaOneShotConfig
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

struct BdmaOneShot
{
  struct BdmaBase base;

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
#endif /* HALM_PLATFORM_STM32_BDMA_ONESHOT_H_ */
