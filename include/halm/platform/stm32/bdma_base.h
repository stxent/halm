/*
 * halm/platform/stm32/bdma_base.h
 * Copyright (C) 2018, 2020, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BDMA_BASE_H_
#define HALM_PLATFORM_STM32_BDMA_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/irq.h>
#include <halm/platform/stm32/dma.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/bdma_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const BdmaBase;

struct BdmaBaseConfig
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

struct BdmaBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *, enum Result);

  /* Precalculated value of the channel configuration register */
  uint32_t config;
  /* IRQ number */
  IrqNumber irq;
  /* Identifier of the stream */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

const struct BdmaBase *bdmaGetInstance(uint8_t);
void bdmaResetInstance(uint8_t);
bool bdmaSetInstance(uint8_t, struct BdmaBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_BDMA_BASE_H_ */
