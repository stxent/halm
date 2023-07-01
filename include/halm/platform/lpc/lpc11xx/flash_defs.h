/*
 * halm/platform/lpc/lpc11xx/flash_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11XX_FLASH_DEFS_H_
#define HALM_PLATFORM_LPC_LPC11XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define CODE_LPC1110_1      0x0A07102BUL
#define CODE_LPC1110_2      0x1A07102BUL

#define CODE_LPC1111_002_1  0x0A16D02BUL
#define CODE_LPC1111_002_2  0x1A16D02BUL
#define CODE_LPC1111_101_1  0x041E502BUL
#define CODE_LPC1111_201_1  0x0416502BUL
#define CODE_LPC1111_101_2  0x2516D02BUL
#define CODE_LPC1111_201_2  0x2516902BUL
#define CODE_LPC1111_103    0x00010013UL
#define CODE_LPC1111_203    0x00010012UL

#define CODE_LPC1112_101_1  0x042D502BUL
#define CODE_LPC1112_201_1  0x0425502BUL
#define CODE_LPC1112_101_2  0x2524D02BUL
#define CODE_LPC1112_201_2  0x2524902BUL
#define CODE_LPC1112_102_1  0x0A24902BUL
#define CODE_LPC1112_102_2  0x1A24902BUL
#define CODE_LPC1112_103    0x00020023UL
#define CODE_LPC1112_203    0x00020022UL

#define CODE_LPC1113_201_1  0x0434502BUL
#define CODE_LPC1113_301_1  0x0434102BUL
#define CODE_LPC1113_301_2  0x2532102BUL
#define CODE_LPC1113_201_2  0x2532902BUL
#define CODE_LPC1113_203    0x00030032UL
#define CODE_LPC1113_303    0x00030030UL

#define CODE_LPC1114_102_1  0x0A40902BUL
#define CODE_LPC1114_102_2  0x1A40902BUL
#define CODE_LPC1114_201_1  0x0444502BUL
#define CODE_LPC1114_301_1  0x0444102BUL
#define CODE_LPC1114_201_2  0x2540902BUL
#define CODE_LPC1114_301_2  0x2540102BUL
#define CODE_LPC1114_203    0x00040042UL
#define CODE_LPC1114_303    0x00040040UL
#define CODE_LPC1114_323    0x00040060UL
#define CODE_LPC1114_333    0x00040070UL

#define CODE_LPC1115_303    0x00050080UL

#define CODE_LPC11C12_301   0x1421102BUL
#define CODE_LPC11C14_301   0x1440102BUL
#define CODE_LPC11C22_301   0x1431102BUL
#define CODE_LPC11C24_301   0x1430102BUL

#define IS_LPC1110(id)      (((id) & 0x0FFF0000UL) == 0x0A070000UL)
#define IS_LPC1111(id)      (((id) & 0xFFFF0000UL) == 0x00010000UL\
    || ((id) & 0x0FFF0000UL) == 0x0A160000UL\
    || ((id) & 0xFFF00000UL) == 0x04100000UL\
    || ((id) & 0xFFFF0000UL) == 0x25160000UL)
#define IS_LPC1112(id)      (((id) & 0xFFFF0000UL) == 0x00020000UL\
    || ((id) & 0x0FFF0000UL) == 0x0A240000UL\
    || ((id) & 0xFFF00000UL) == 0x04200000UL\
    || ((id) & 0xFFFF0000UL) == 0x25240000UL)
#define IS_LPC1113(id)      (((id) & 0xFFFF0000UL) == 0x00030000UL\
    || ((id) & 0xFFF00000UL) == 0x04300000UL\
    || ((id) & 0xFFFF0000UL) == 0x25320000UL)
#define IS_LPC1114(id)      (((id) & 0xFFFF0070UL) == 0x00040040UL\
    || ((id) & 0x0FFF0000UL) == 0x0A400000UL\
    || ((id) & 0xFFF00000UL) == 0x04400000UL\
    || ((id) & 0xFFFF0000UL) == 0x25400000UL)
#define IS_LPC1115(id)      (((id) & 0xFFFF0000UL) == 0x00050000UL)
#define IS_LPC11CXX(id)     (((id) & 0xFF000000UL) == 0x14000000UL)
#define IS_LPC11CX2(id)     (((id) & 0xFF010000UL) == 0x14010000UL)
#define IS_LPC11CX4(id)     (((id) & 0xFF010000UL) == 0x14000000UL)
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
static inline uint32_t addressToSector(uint32_t address,
    bool uniform __attribute__((unused)))
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
static inline bool isSectorPositionValid(uint32_t position,
    bool uniform __attribute__((unused)))
{
  return (position & (FLASH_SECTOR_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_FLASH_DEFS_H_ */
