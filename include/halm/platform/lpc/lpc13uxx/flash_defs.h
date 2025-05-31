/*
 * halm/platform/lpc/lpc13uxx/flash_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13UXX_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC13UXX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1315      0x3A010523UL
#define CODE_LPC1316      0x1A018524UL
#define CODE_LPC1317      0x1A020525UL
#define CODE_LPC1345      0x28010541UL
#define CODE_LPC1346      0x08018542UL
#define CODE_LPC1347      0x08020543UL
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE   256
#define FLASH_SECTOR_SIZE 4096
/*----------------------------------------------------------------------------*/
#define IAP_BASE          0x1FFF1FF1UL
/*----------------------------------------------------------------------------*/
static inline uint8_t addressToBank(uint32_t)
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPage(uint32_t address)
{
  return address / FLASH_PAGE_SIZE;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSector(uint32_t address, bool)
{
  return address / FLASH_SECTOR_SIZE;
}
/*----------------------------------------------------------------------------*/
static inline bool isPagePositionValid(uint32_t position)
{
  return (position & (FLASH_PAGE_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(uint32_t position, bool)
{
  return (position & (FLASH_SECTOR_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_FLASH_DEFS_H_ */
