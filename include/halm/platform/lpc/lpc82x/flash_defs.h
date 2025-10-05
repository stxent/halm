/*
 * halm/platform/lpc/lpc82x/flash_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC82X_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC824       0x00008220UL
#define CODE_LPC822       0x00008240UL
#define CODE_LPC82X_MASK  0x0000FFF0UL
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE   64
#define FLASH_SECTOR_SIZE 1024
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
#endif /* HALM_PLATFORM_LPC_LPC82X_FLASH_DEFS_H_ */
