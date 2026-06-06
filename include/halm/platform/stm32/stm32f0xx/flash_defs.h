/*
 * halm/platform/stm32/stm32f0xx/flash_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FLASH_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_FLASH_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F0XX_FLASH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Debug ID Code register------------------------------------*/
enum
{
  DEV_ID_STM32F03X  = 0x444,
  DEV_ID_STM32F04X  = 0x445,
  DEV_ID_STM32F05X  = 0x440,
  DEV_ID_STM32F07X  = 0x448,
  DEV_ID_STM32F09X  = 0x442
};

#define IDCODE_DEV_ID_MASK              BIT_FIELD(MASK(12), 0)
#define IDCODE_DEV_ID_VALUE(reg) \
    FIELD_VALUE((reg), IDCODE_DEV_ID_MASK, 0)
/*------------------Flash Access Control Register-----------------------------*/
#define FLASH_ACR_LATENCY(value)        BIT_FIELD((value), 0)
#define FLASH_ACR_LATENCY_MASK          BIT_FIELD(MASK(3), 0)
#define FLASH_ACR_LATENCY_VALUE(reg) \
    FIELD_VALUE((reg), FLASH_ACR_LATENCY_MASK, 0)

#define FLASH_ACR_PRFTBE                BIT(4)
#define FLASH_ACR_PRFTBS                BIT(5)
/*------------------Flash Key Register----------------------------------------*/
#define KEYR_KEY1                       0x45670123UL
#define KEYR_KEY2                       0xCDEF89ABUL
/*------------------Flash Option Key Register---------------------------------*/
#define OPTKEYR_KEY1                    0x08192A3BUL
#define OPTKEYR_KEY2                    0x4C5D6E7FUL
/*------------------Flash Status Register-------------------------------------*/
#define SR_BSY                          BIT(0)
#define SR_PGERR                        BIT(2)
#define SR_WRPRTERR                     BIT(4)
#define SR_EOP                          BIT(5)
/*------------------Flash Control Register------------------------------------*/
/* Programming enable */
#define CR_PG                           BIT(0)
/* Page erase bit */
#define CR_PER                          BIT(1)
/* Mass Erase bit */
#define CR_MER                          BIT(2)
/* Option byte programming */
#define CR_OPTPG                        BIT(4)
/* Option byte erase */
#define CR_OPTER                        BIT(5)
/* Erase start */
#define CR_STRT                         BIT(6)
/* Lock bit */
#define CR_LOCK                         BIT(7)
/* Option bytes write enable */
#define CR_OPTWRE                       BIT(9)
/* Error interrupt enable bit */ 
#define CR_ERRIE                        BIT(10)
/* End-of-operation interrupt enable */
#define CR_EOPIE                        BIT(12)
/* Force option byte loading */
#define CR_OBL_LAUNCH                   BIT(13)
/*----------------------------------------------------------------------------*/
#define FLASH_USE_PAGES

#define FLASH_BASE_ADDRESS              0x08000000UL
#define FLASH_PAGE_SIZE_LARGE           (2 * 1024)
#define FLASH_PAGE_SIZE_SMALL           (1 * 1024)
/*----------------------------------------------------------------------------*/
static inline uint32_t addressToPageSize(bool large)
{
  return large ? FLASH_PAGE_SIZE_LARGE : FLASH_PAGE_SIZE_SMALL;
}
/*----------------------------------------------------------------------------*/
static inline bool isPagePositionValid(bool large, uint32_t position)
{
  if (large)
    return (position & (FLASH_PAGE_SIZE_LARGE - 1)) == 0;
  else
    return (position & (FLASH_PAGE_SIZE_SMALL - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_FLASH_DEFS_H_ */
