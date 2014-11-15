/*
 * platform/nxp/sdmmc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDMMC_BASE_H_
#define PLATFORM_NXP_SDMMC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const SdmmcBase;
/*----------------------------------------------------------------------------*/
struct SdmmcBaseConfig
{

};
/*----------------------------------------------------------------------------*/
#define MMC_SECTOR_SIZE 512
/*----------------------------------------------------------------------------*/
struct SdmmcBase
{
  struct Interface parent;

  /* Current position in internal memory space */
  uint64_t position;

  uint32_t response[4];
  uint32_t cid[4];
  uint32_t csd[4];
  uint32_t ext_csd[MMC_SECTOR_SIZE / 4];
  uint32_t card_type;
  uint32_t rca;
  uint32_t speed;
  uint32_t block_len;
  uint32_t device_size;
  uint32_t blocknr;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDMMC_BASE_H_ */
