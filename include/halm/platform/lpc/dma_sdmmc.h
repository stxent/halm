/*
 * halm/platform/lpc/dma_sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_DMA_SDMMC_H_
#define HALM_PLATFORM_LPC_DMA_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/sdmmc.h>
/*----------------------------------------------------------------------------*/
struct DmaSdmmcEntry
{
  uint32_t control;
  uint32_t size;
  uint32_t buffer1;
  uint32_t buffer2;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaSdmmc;

struct DmaSdmmcConfig
{
  const struct Sdmmc *parent;
  /** Mandatory: number of blocks in the chain. */
  size_t number;
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum DmaBurst burst;
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
