/*
 * halm/platform/lpc/dma_sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_DMA_SDMMC_H_
#define HALM_PLATFORM_LPC_DMA_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gpdma_base.h>
#include <halm/platform/lpc/sdmmc.h>
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] DmaSdmmcEntry
{
  uint32_t control;
  uint32_t size;
  uint32_t buffer1;
  uint32_t buffer2;
};
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaSdmmc;

struct DmaSdmmcConfig
{
  const struct Sdmmc *parent;
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum GpDmaBurst burst;
  /** Mandatory: number of blocks in the chain. */
  size_t number;
};

struct DmaSdmmc
{
  struct Dma base;

  void *reg;

  /* Descriptor list container */
  struct DmaSdmmcEntry *list;

  /* List capacity */
  size_t capacity;
  /* Current list length */
  size_t length;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_DMA_SDMMC_H_ */
