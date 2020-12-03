/*
 * halm/platform/lpc/lpc11exx/flash_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC11EXX_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC11EXX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC11E11_101     0x293E902BUL
#define CODE_LPC11E12_201     0x2954502BUL
#define CODE_LPC11E13_301     0x296A102BUL
#define CODE_LPC11E14_401     0x2980102BUL

#define CODE_LPC11E36_501     0x00009C41UL
#define CODE_LPC11E37_401     0x00007C45UL
#define CODE_LPC11E37_501     0x00007C41UL

#define CODE_LPC11E66         0x0000DCC1UL
#define CODE_LPC11E67         0x0000BC81UL
#define CODE_LPC11E68         0x00007C01UL

#define CODE_LPC11U66         0x0000DCC8UL
#define CODE_LPC11U67_1       0x0000BC80UL
#define CODE_LPC11U67_2       0x0000BC88UL
#define CODE_LPC11U68_1       0x00007C00UL
#define CODE_LPC11U68_2       0x00007C08UL
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE       256
#define FLASH_SECTOR_SIZE_0   4096
#define FLASH_SECTOR_SIZE_1   32768
#define FLASH_SECTORS_BORDER  0x18000
#define FLASH_SECTORS_OFFSET  (FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0)
/*----------------------------------------------------------------------------*/
#define IAP_BASE              0x1FFF1FF1UL
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
    /* Sectors from 0 to 23 have size of 4 kB */
    return address / FLASH_SECTOR_SIZE_0;
  }
  else
  {
    /* Sectors from 24 to 28 have size of 32 kB */
    return (address - FLASH_SECTORS_BORDER) / FLASH_SECTOR_SIZE_1
        + FLASH_SECTORS_OFFSET;
  }
}
/*----------------------------------------------------------------------------*/
static inline bool isPagePositionValid(uint32_t position)
{
  return (position & (FLASH_PAGE_SIZE - 1)) == 0;
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
#endif /* HALM_PLATFORM_LPC_LPC11EXX_FLASH_DEFS_H_ */
