/*
 * halm/platform/lpc/lpc43xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC4350            0xA0000830UL
#define CODE_LPC4330            0xA0000A30UL
#define CODE_LPC4320            0xA000CB3CUL
#define CODE_LPC4310            0xA00ACB3FUL
#define CODE_LPC4370FET256      0x00000030UL
#define CODE_LPC4370FET100      0x00000230UL

#define CODE_LPC435X            0xA001C830UL
#define CODE_LPC433X            0xA001CA30UL
#define CODE_LPC4325_17         0xA001CB3CUL
#define CODE_LPC4322_13         0xA00BCB3CUL
#define CODE_LPC4315_17         0xA001CB3FUL
#define CODE_LPC4312_13         0xA00BCB3FUL

#define FLASH_AVAILABLE         0x00010000UL
#define FLASH_SIZE_MASK         0x000000FFUL
#define FLASH_SIZE_256_256      0x00000044UL
#define FLASH_SIZE_384_384      0x00000022UL
#define FLASH_SIZE_512_0        0x00000080UL
#define FLASH_SIZE_512_512      0x00000000UL
/*----------------------------------------------------------------------------*/
#define FLASH_BANK_A_ADDRESS    0x1A000000UL
#define FLASH_BANK_B_ADDRESS    0x1B000000UL
#define FLASH_BANK_BORDER       0x80000
#define FLASH_BANK_MASK         (FLASH_BANK_BORDER - 1)
#define FLASH_PAGE_SIZE         512
#define FLASH_SECTOR_SIZE_0     8192
#define FLASH_SECTOR_SIZE_1     65536
#define FLASH_SECTORS_BORDER    0x10000
#define FLASH_SECTORS_OFFSET \
    (FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0 \
        - FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_1)
/*----------------------------------------------------------------------------*/
#define IAP_BASE                (*(volatile uint32_t *)0x10400100UL)
/*----------------------------------------------------------------------------*/
static inline uint8_t addressToBank(uint32_t address)
{
  return address < FLASH_BANK_B_ADDRESS ? 0 : 1;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPage(uint32_t address)
{
  return address;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSector(uint32_t address,
    [[maybe_unused]] bool uniform)
{
  const uint32_t local = address & FLASH_BANK_MASK;

  if (local >= FLASH_SECTORS_BORDER)
  {
    /* Sectors from 8 to 14 have size of 64 kB */
    return local / FLASH_SECTOR_SIZE_1 + FLASH_SECTORS_OFFSET;
  }

  /* Sectors from 0 to 7 have size of 8 kB */
  return local / FLASH_SECTOR_SIZE_0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_FLASH_DEFS_H_ */
