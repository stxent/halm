/*
 * halm/platform/nxp/lpc11xx/flash_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC11XX_FLASH_DEFS_H_
#define HALM_PLATFORM_NXP_LPC11XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1110_1      0x0A07102BU
#define CODE_LPC1110_2      0x1A07102BU

#define CODE_LPC1111_002_1  0x0A16D02BU
#define CODE_LPC1111_002_2  0x1A16D02BU
#define CODE_LPC1111_101_1  0x041E502BU
#define CODE_LPC1111_201_1  0x0416502BU
#define CODE_LPC1111_101_2  0x2516D02BU
#define CODE_LPC1111_201_2  0x2516902BU
#define CODE_LPC1111_103    0x00010013U
#define CODE_LPC1111_203    0x00010012U

#define CODE_LPC1112_101_1  0x042D502BU
#define CODE_LPC1112_201_1  0x0425502BU
#define CODE_LPC1112_101_2  0x2524D02BU
#define CODE_LPC1112_201_2  0x2524902BU
#define CODE_LPC1112_102_1  0x0A24902BU
#define CODE_LPC1112_102_2  0x1A24902BU
#define CODE_LPC1112_103    0x00020023U
#define CODE_LPC1112_203    0x00020022U

#define CODE_LPC1113_201_1  0x0434502BU
#define CODE_LPC1113_301_1  0x0434102BU
#define CODE_LPC1113_301_2  0x2532102BU
#define CODE_LPC1113_201_2  0x2532902BU
#define CODE_LPC1113_203    0x00030032U
#define CODE_LPC1113_303    0x00030030U

#define CODE_LPC1114_102_1  0x0A40902BU
#define CODE_LPC1114_102_2  0x1A40902BU
#define CODE_LPC1114_201_1  0x0444502BU
#define CODE_LPC1114_301_1  0x0444102BU
#define CODE_LPC1114_201_2  0x2540902BU
#define CODE_LPC1114_301_2  0x2540102BU
#define CODE_LPC1114_203    0x00040042U
#define CODE_LPC1114_303    0x00040040U
#define CODE_LPC1114_323    0x00040060U
#define CODE_LPC1114_333    0x00040070U

#define CODE_LPC1115_303    0x00050080U

#define CODE_LPC11C12_301   0x1421102BU
#define CODE_LPC11C14_301   0x1440102BU
#define CODE_LPC11C22_301   0x1431102BU
#define CODE_LPC11C24_301   0x1430102BU

#define IS_LPC1110(id)      (((id) & 0x0FFF0000U) == 0x0A070000U)
#define IS_LPC1111(id)      (((id) & 0xFFFF0000U) == 0x00010000U\
    || ((id) & 0x0FFF0000U) == 0x0A160000U\
    || ((id) & 0xFFF00000U) == 0x04100000U\
    || ((id) & 0xFFFF0000U) == 0x25160000U)
#define IS_LPC1112(id)      (((id) & 0xFFFF0000U) == 0x00020000U\
    || ((id) & 0x0FFF0000U) == 0x0A240000U\
    || ((id) & 0xFFF00000U) == 0x04200000U\
    || ((id) & 0xFFFF0000U) == 0x25240000U)
#define IS_LPC1113(id)      (((id) & 0xFFFF0000U) == 0x00030000U\
    || ((id) & 0xFFF00000U) == 0x04300000U\
    || ((id) & 0xFFFF0000U) == 0x25320000U)
#define IS_LPC1114(id)      (((id) & 0xFFFF0000U) == 0x00040000U\
    || ((id) & 0x0FFF0000U) == 0x0A400000U\
    || ((id) & 0xFFF00000U) == 0x04400000U\
    || ((id) & 0xFFFF0000U) == 0x25400000U)
#define IS_LPC1115(id)      (((id) & 0xFFFF0000U) == 0x00050000U)
#define IS_LPC11CXX(id)     (((id) & 0xFF000000U) == 0x14000000U)
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
static inline uint32_t addressToPage(uint32_t address)
{
  return address / FLASH_PAGE_SIZE;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSector(uint32_t address)
{
  return address / FLASH_SECTOR_SIZE;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11XX_FLASH_DEFS_H_ */
