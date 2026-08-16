/*
 * halm/platform/stm32/fsmc_sram.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FSMC_SRAM_H_
#define HALM_PLATFORM_STM32_FSMC_SRAM_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const FsmcSram;

struct FsmcSramConfig
{
  struct
  {
    /** Optional: bus turnaround time (ns). */
    uint32_t turnaround;

    /** Mandatory: delay from address change to output enable (ns). */
    uint32_t oe;
    /** Mandatory: delay from chip select to read access (ns). */
    uint32_t rd;
    /** Mandatory: delay from chip select to write enable (ns). */
    uint32_t we;
    /** Mandatory: delay from chip select to write access (ns). */
    uint32_t wr;
  } timings;

  struct {
    /** Mandatory: address bus width in bits. */
    uint8_t address;
    /** Mandatory: data bus width in bits. */
    uint8_t data;
  } width;

  /** Optional: pin signaling slew rate. */
  enum PinSlewRate speed;
  /** Mandatory: subbank index within Bank 1. */
  uint8_t subbank;

  /** Optional: enable both Write Enable and Byte Lane Select signals. */
  bool useWriteEnable;
};

struct FsmcSram
{
  struct Entity base;

  /* Starting address of the memory */
  void *address;
  /* Subbank index */
  uint8_t subbank;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void *fsmcSramAddress(const struct FsmcSram *memory)
{
  return memory->address;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_FSMC_SRAM_H_ */
