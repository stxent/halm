/*
 * halm/platform/lpc/emc_sdram.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_EMC_SDRAM_H_
#define HALM_PLATFORM_LPC_EMC_SDRAM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const EmcSdram;

struct EmcSdramConfig
{
  struct
  {
    /** Optional: clock delay, ps. */
    uint32_t delay;
    /** Mandatory: refresh time, ns. */
    uint32_t refresh;

    /** Mandatory: last data out to active time, ns. */
    uint32_t apr;
    /** Mandatory: load mode register to active command time, ns. */
    uint32_t mrd;
    /** Mandatory: active to precharge command period, ns. */
    uint32_t ras;
    /** Mandatory: active to active command period, ns. */
    uint32_t rc;
    /** Mandatory: active bank A to active bank B time, ns. */
    uint32_t rrd;
    /** Mandatory: precharge command period, ns. */
    uint32_t rp;
    /** Mandatory: write recovery time, ns. */
    uint32_t wr;
    /** Mandatory: exit self refresh to active command, ns. */
    uint32_t xsr;
  } timings;

  struct {
    /** Mandatory: total data bus width. */
    uint8_t bus;
    /** Mandatory: data bus width of a single device. */
    uint8_t device;
  } width;

  /** Mandatory: clock pins used by memory devices. */
  bool clocks[4];

  /** Mandatory: channel number. */
  uint8_t channel;
  /** Mandatory: CAS latency, ticks. */
  uint8_t latency;

  /** Mandatory: number of memory banks. */
  uint8_t banks;
  /** Mandatory: number of columns, power of two. */
  uint8_t columns;
  /** Mandatory: number of rows, power of two. */
  uint8_t rows;
};

struct EmcSdram
{
  struct Entity base;

  /* Starting address of the memory bank */
  void *address;
  /* Memory size */
  uint32_t size;
  /* Peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *emcSdramAddress(const struct EmcSdram *memory)
{
  return memory->address;
}

static inline size_t emcSdramSize(const struct EmcSdram *memory)
{
  return (size_t)memory->size;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_EMC_SDRAM_H_ */
