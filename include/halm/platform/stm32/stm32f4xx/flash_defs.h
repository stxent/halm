/*
 * halm/platform/stm32/stm32f4xx/flash_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_FLASH_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F4XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Flash Access Control Register-----------------------------*/
#define FLASH_ACR_LATENCY(value)        BIT_FIELD((value), 0)
#define FLASH_ACR_LATENCY_MASK          BIT_FIELD(MASK(4), 0)
#define FLASH_ACR_LATENCY_VALUE(reg) \
    FIELD_VALUE((reg), FLASH_ACR_LATENCY_MASK, 0)

#define FLASH_ACR_PRFTEN                BIT(8)
#define FLASH_ACR_ICEN                  BIT(9)
#define FLASH_ACR_DCEN                  BIT(10)
#define FLASH_ACR_ICRST                 BIT(11)
#define FLASH_ACR_DCRST                 BIT(12)
/*------------------Flash Key Register----------------------------------------*/
#define KEYR_KEY1                       0x45670123UL
#define KEYR_KEY2                       0xCDEF89ABUL
/*------------------Flash Option Key Register---------------------------------*/
#define OPTKEYR_KEY1                    0x08192A3BUL
#define OPTKEYR_KEY2                    0x4C5D6E7FUL
/*------------------Flash Status Register-------------------------------------*/
#define SR_EOP                          BIT(0)
#define SR_OPERR                        BIT(1)
#define SR_WRPERR                       BIT(4)
#define SR_PGAERR                       BIT(5)
#define SR_PGPERR                       BIT(6)
#define SR_PGSERR                       BIT(7)
#define SR_RDERR                        BIT(8)
#define SR_BSY                          BIT(16)
/*------------------Flash Control Register------------------------------------*/
enum
{
  PSIZE_X8  = 0,
  PSIZE_X16 = 1,
  PSIZE_X32 = 2,
  PSIZE_X64 = 3
};

/* Programming enable */
#define CR_PG                           BIT(0)
/* Sector erase bit */
#define CR_SER                          BIT(1)
/* Mass Erase of Bank 1 */
#define CR_MER                          BIT(2)

/* Sector number selection */
#define CR_SNB(value)                   BIT_FIELD((value), 3)
#define CR_SNB_MASK                     BIT_FIELD(MASK(4), 3)
#define CR_SNB_VALUE(reg)               FIELD_VALUE((reg), CR_SNB_MASK, 3)

/* Programming parallelism */
#define CR_PSIZE(value)                 BIT_FIELD((value), 8)
#define CR_PSIZE_MASK                   BIT_FIELD(MASK(2), 8)
#define CR_PSIZE_VALUE(reg)             FIELD_VALUE((reg), CR_PSIZE_MASK, 8)

/* Mass Erase of Bank 2 */
#define CR_MER1                         BIT(15)
/* Erase start */
#define CR_STRT                         BIT(16)
/* End-of-operation interrupt enable */
#define CR_EOPIE                        BIT(24)
/* Error interrupt enable bit */ 
#define CR_ERRIE                        BIT(25)
/* Lock bit */
#define CR_LOCK                         BIT(31)
/*----------------------------------------------------------------------------*/
#define FLASH_USE_SECTORS

#define FLASH_BASE_ADDRESS              0x08000000UL
#define FLASH_SECTOR_SIZE_0             (16 * 1024)
#define FLASH_SECTOR_SIZE_1             (64 * 1024)
#define FLASH_SECTOR_SIZE_2             (64 * 1024)
#define FLASH_SECTORS_BORDER_0          (128 * 1024)
#define FLASH_SECTORS_BORDER_1          (256 * 1024)
#define FLASH_SECTORS_COUNT_0 \
    (FLASH_SECTORS_BORDER_0 / FLASH_SECTOR_SIZE_0)
#define FLASH_SECTORS_COUNT_1 \
    ((FLASH_SECTORS_BORDER_1 - FLASH_SECTORS_BORDER_0) / FLASH_SECTOR_SIZE_1)
/*----------------------------------------------------------------------------*/
static inline unsigned int addressToBank(uint32_t)
{
  // TODO STM32F4 add bank support
  return 0;
}
/*----------------------------------------------------------------------------*/
static inline unsigned int addressToSector(uint32_t address)
{
  if (address >= FLASH_SECTORS_BORDER_1)
  {
    /* Sectors from 5 to 11 have size of 128 kB */
    return FLASH_SECTORS_COUNT_0 + FLASH_SECTORS_COUNT_1
        + (address - FLASH_SECTORS_BORDER_1) / FLASH_SECTOR_SIZE_2;
  }
  if (address >= FLASH_SECTORS_BORDER_0)
  {
    /* Size of the sector 4 is 128 kB */
    return FLASH_SECTORS_COUNT_0
        + (address - FLASH_SECTORS_BORDER_0) / FLASH_SECTOR_SIZE_1;
  }

  /* Sectors from 0 to 3 have size of 16 kB */
  return address / FLASH_SECTOR_SIZE_0;
}
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToSectorSize(uint32_t address)
{
  if (address >= FLASH_SECTORS_BORDER_1)
  {
    /* Sectors from 5 to 11 have size of 128 kB */
    return FLASH_SECTOR_SIZE_2;
  }
  if (address >= FLASH_SECTORS_BORDER_0)
  {
    /* Size of the sector 4 is 128 kB */
    return FLASH_SECTOR_SIZE_1;
  }

  /* Sectors from 0 to 3 have size of 16 kB */
  return FLASH_SECTOR_SIZE_0;
}
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(uint32_t position)
{
  if (position < FLASH_SECTORS_BORDER_0)
    return (position & (FLASH_SECTOR_SIZE_0 - 1)) == 0;
  if (position < FLASH_SECTORS_BORDER_1)
    return (position & (FLASH_SECTOR_SIZE_1 - 1)) == 0;
  return (position & (FLASH_SECTOR_SIZE_2 - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_FLASH_DEFS_H_ */
