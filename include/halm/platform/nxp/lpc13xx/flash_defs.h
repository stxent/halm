/*
 * halm/platform/nxp/lpc13xx/flash_defs.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_
#define HALM_PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1311      0x2C42502BU
#define CODE_LPC1311_01   0x1816902BU
#define CODE_LPC1313      0x2C40102BU
#define CODE_LPC1313_01   0x1830102BU
#define CODE_LPC1342      0x3D01402BU
#define CODE_LPC1343      0x3D00002BU
/*----------------------------------------------------------------------------*/
#define FLASH_PAGE_SIZE   256
#define FLASH_SECTOR_SIZE 4096
/*----------------------------------------------------------------------------*/
#define IAP_BASE          0x1FFF1FF1U
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
#endif /* HALM_PLATFORM_NXP_LPC13XX_FLASH_DEFS_H_ */
