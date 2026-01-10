/*
 * halm/platform/bouffalo/dma_oneshot.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_DMA_ONESHOT_H_
#define HALM_PLATFORM_BOUFFALO_DMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/dma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaOneShot;

struct DmaOneShotConfig
{
  /** Mandatory: request connection to the peripheral or memory. */
  enum DmaEvent event;
  /** Mandatory: transfer type. */
  enum DmaType type;
  /** Mandatory: channel number. */
  uint8_t channel;
};

struct DmaOneShot
{
  struct DmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Control register value */
  uint32_t control;
  /* The destination address of the data to be transferred */
  uintptr_t destination;
  /* The source address of the data */
  uintptr_t source;

  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_DMA_ONESHOT_H_ */
