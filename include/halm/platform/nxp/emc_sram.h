/*
 * halm/platform/nxp/emc_sram.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_EMC_SRAM_H_
#define HALM_PLATFORM_NXP_EMC_SRAM_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
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

  struct
  {
    /** Mandatory: read cycle time, ns. */
    uint32_t rc;
    /** Mandatory: address access time, ns. */
    uint32_t oe;
    /** Mandatory: write cycle time, ns. */
    uint32_t wc;
    /** Mandatory: write enable time, ns. */
    uint32_t we;
  } timings;

  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: use Byte Lane Select signals along with Write Enable signal. */
  bool partitioned;
};
/*----------------------------------------------------------------------------*/
struct EmcSram
{
  struct Entity base;

  /* Starting address of the memory bank */
  void *address;
  /* Peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
static inline void *emcSramAddress(const struct EmcSram *memory)
{
  return memory->address;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_EMC_SRAM_H_ */
