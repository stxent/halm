/*
 * platform/nxp/dma_sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_DMA_SDMMC_H_
#define PLATFORM_NXP_DMA_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <platform/nxp/sdmmc.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const DmaSdmmc;
/*----------------------------------------------------------------------------*/
struct DmaSdmmcConfig
{
  const struct Sdmmc *parent;
  /** Mandatory: number of blocks in the chain. */
  uint16_t number;
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum dmaBurst burst;
};
/*----------------------------------------------------------------------------*/
struct DmaSdmmcEntry
{
  uint32_t control;
  uint32_t size;
  uint32_t buffer1;
  uint32_t buffer2;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct DmaSdmmc
{
  struct Dma parent;

  void *reg;

  /* Descriptor list container */
  struct DmaSdmmcEntry *list;

  /* List capacity */
  uint16_t capacity;
  /* Current list length */
  uint16_t length;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_DMA_SDMMC_H_ */
