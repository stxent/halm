/*
 * platform/nxp/lpc43xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_FLASH_DEFS_H_
#define HALM_PLATFORM_NXP_LPC43XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <bits.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC4350            0xA0000830
#define CODE_LPC4330            0xA0000A30
#define CODE_LPC4320            0xA000CB3C
#define CODE_LPC4310            0xA00ACB3F
#define CODE_LPC4370FET256      0x00000030
#define CODE_LPC4370FET100      0x00000230

#define CODE_LPC435X            0xA001C830
#define CODE_LPC433X            0xA001CA30
#define CODE_LPC4325_17         0xA001CB3C
#define CODE_LPC4322_13         0xA00BCB3C
#define CODE_LPC4315_17         0xA001CB3F
#define CODE_LPC4312_13         0xA00BCB3F

#define FLASH_AVAILABLE         0x00010000
#define FLASH_SIZE_MASK         0x000000FF
#define FLASH_SIZE_256_256      0x00000044
#define FLASH_SIZE_384_384      0x00000022
#define FLASH_SIZE_512_0        0x00000080
#define FLASH_SIZE_512_512      0x00000000
/*----------------------------------------------------------------------------*/
#define FLASH_BANK_A            0x1A000000
#define FLASH_BANK_B            0x1B000000
#define FLASH_BANK_MASK         0x0007FFFF
#define FLASH_PAGE_SIZE         512
#define FLASH_SECTOR_SIZE_0     8192
#define FLASH_SECTOR_SIZE_1     65536
#define FLASH_SECTORS_BORDER    0x10000
#define FLASH_SECTORS_OFFSET    (FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0 \
    - FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_1)
/*----------------------------------------------------------------------------*/
#define IAP_BASE                (*(volatile uint32_t *)0x10400100)
/*----------------------------------------------------------------------------*/
#define FLASH_SIZE_ENCODE(a, b) \
    (BIT_FIELD((a) >> 10, 0) | BIT_FIELD((b) >> 10, 16))
#define FLASH_SIZE_DECODE_A(value) \
    (FIELD_VALUE(value, BIT_FIELD(MASK(16), 0), 0) << 10)
#define FLASH_SIZE_DECODE_B(value) \
    (FIELD_VALUE(value, BIT_FIELD(MASK(16), 16), 16) << 10)
/*----------------------------------------------------------------------------*/
static inline uint8_t addressToBank(uint32_t address)
{
  return address < FLASH_BANK_B ? 0 : 1;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPage(uint32_t address)
{
  return (address & FLASH_BANK_MASK) / FLASH_PAGE_SIZE;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSector(uint32_t address)
{
  const uint32_t localAddress = address & FLASH_BANK_MASK;

  if (localAddress < FLASH_SECTORS_BORDER)
  {
    /* Sectors from 0 to 7 have size of 8 kB */
    return localAddress / FLASH_SECTOR_SIZE_0;
  }
  else
  {
    /* Sectors from 8 to 14 have size of 64 kB */
    return localAddress / FLASH_SECTOR_SIZE_1 + FLASH_SECTORS_OFFSET;
  }
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_FLASH_DEFS_H_ */
