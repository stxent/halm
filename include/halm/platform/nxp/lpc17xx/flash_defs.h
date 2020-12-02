/*
 * halm/platform/nxp/lpc17xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_
#define HALM_PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1769            0x26113F37UL
#define CODE_LPC1768            0x26013F37UL
#define CODE_LPC1767            0x26012837UL
#define CODE_LPC1766            0x26013F33UL
#define CODE_LPC1765            0x26013733UL
#define CODE_LPC1764            0x26011922UL
#define CODE_LPC1763            0x26012033UL
#define CODE_LPC1759            0x25113737UL
#define CODE_LPC1758            0x25013F37UL
#define CODE_LPC1756            0x25011723UL
#define CODE_LPC1754            0x25011722UL
#define CODE_LPC1752            0x25001121UL
#define CODE_LPC1751            0x25001118UL
#define CODE_LPC1751_00         0x25001110UL
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE         256
#define FLASH_SECTOR_SIZE_0     4096
#define FLASH_SECTOR_SIZE_1     32768
#define FLASH_SECTORS_BORDER    0x10000
#define FLASH_SECTORS_OFFSET    (FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0)
/*----------------------------------------------------------------------------*/
#define IAP_BASE                0x1FFF1FF1UL
/*----------------------------------------------------------------------------*/
static inline uint8_t addressToBank(uint32_t address __attribute__((unused)))
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPage(uint32_t address __attribute__((unused)))
{
  return 0;
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
    return (address - FLASH_SECTORS_BORDER) / FLASH_SECTOR_SIZE_1
        + FLASH_SECTORS_OFFSET;
  }
}
/*----------------------------------------------------------------------------*/
static inline bool isPagePositionValid(uint32_t position
    __attribute__((unused)))
{
  return false;
}
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(uint32_t position)
{
  if (position < FLASH_SECTORS_BORDER)
    return (position & (FLASH_SECTOR_SIZE_0 - 1)) == 0;
  else
    return (position & (FLASH_SECTOR_SIZE_1 - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_FLASH_DEFS_H_ */
