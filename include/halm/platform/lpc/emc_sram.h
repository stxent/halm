/*
 * halm/platform/lpc/emc_sram.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_EMC_SRAM_H_
#define HALM_PLATFORM_LPC_EMC_SRAM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EmcSram;

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

struct EmcSram
{
  struct Entity base;

  /* Starting address of the memory bank */
  void *address;
  /* Peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *emcSramAddress(const struct EmcSram *memory)
{
  return memory->address;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_EMC_SRAM_H_ */
