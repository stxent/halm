/*
 * platform/nxp/lpc17xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_
#define PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1769            0x26113F37
#define CODE_LPC1768            0x26013F37
#define CODE_LPC1767            0x26012837
#define CODE_LPC1766            0x26013F33
#define CODE_LPC1765            0x26013733
#define CODE_LPC1764            0x26011922
#define CODE_LPC1763            0x26012033
#define CODE_LPC1759            0x25113737
#define CODE_LPC1758            0x25013F37
#define CODE_LPC1756            0x25011723
#define CODE_LPC1754            0x25011722
#define CODE_LPC1752            0x25001121
#define CODE_LPC1751            0x25001118
#define CODE_LPC1751_00         0x25001110
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE         256
#define FLASH_SECTOR_SIZE_0     4096
#define FLASH_SECTOR_SIZE_1     32768
#define FLASH_SECTORS_BORDER    0x10000
#define FLASH_SECTORS_OFFSET    (FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0 \
    - FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_1)
/*----------------------------------------------------------------------------*/
#define IAP_BASE                0x1FFF1FF1U
/*----------------------------------------------------------------------------*/
static inline uint8_t addressToBank(uint32_t address __attribute__((unused)))
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPage(uint32_t address)
{
  return address / FLASH_PAGE_SIZE;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSector(uint32_t address)
{
  if (address < FLASH_SECTORS_BORDER)
  {
    /* Sectors from 0 to 15 have size of 4 kB */
    return address / FLASH_SECTOR_SIZE_0;
  }
  else
  {
    /* Sectors from 16 to 29 have size of 32 kB */
    return address / FLASH_SECTOR_SIZE_1 + FLASH_SECTORS_OFFSET;
  }
}
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_ */
