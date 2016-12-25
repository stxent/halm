/*
 * halm/platform/nxp/emc_sram.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_EMC_SRAM_H_
#define HALM_PLATFORM_NXP_EMC_SRAM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EmcSram;
/*----------------------------------------------------------------------------*/
struct EmcSramConfig
{
  /** Mandatory: address bus width. */
  size_t addressWidth;
  /** Mandatory: data bus width. */
  size_t dataWidth;
  /** Mandatory: chip select number. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct EmcSram
{
  struct Entity base;

  /* Peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_EMC_SRAM_H_ */
