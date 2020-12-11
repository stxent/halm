/*
 * halm/platform/lpc/lpc13xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC13XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1311      0x2C42502BUL
#define CODE_LPC1311_01   0x1816902BUL
#define CODE_LPC1313      0x2C40102BUL
#define CODE_LPC1313_01   0x1830102BUL
#define CODE_LPC1342      0x3D01402BUL
#define CODE_LPC1343      0x3D00002BUL
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE   256
#define FLASH_SECTOR_SIZE 4096
/*----------------------------------------------------------------------------*/
#define IAP_BASE          0x1FFF1FF1UL
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
  return address / FLASH_SECTOR_SIZE;
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
  return (position & (FLASH_SECTOR_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_FLASH_DEFS_H_ */
