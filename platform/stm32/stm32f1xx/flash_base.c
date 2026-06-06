/*
 * flash_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/stm32/flash_base.h>
#include <halm/platform/stm32/flash_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static inline void lockFlash(unsigned int);
static inline void unlockFlash(unsigned int);
static inline enum Result waitForCompletion(unsigned int);

static enum Result flashInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const FlashBase = &(const struct EntityClass){
    .size = sizeof(struct FlashBase),
    .init = flashInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static inline void lockFlash(unsigned int bank)
{
  STM_FLASH->BANK[bank].CR |= CR_LOCK;
}
/*----------------------------------------------------------------------------*/
static inline void unlockFlash(unsigned int bank)
{
  STM_FLASH->BANK[bank].KEYR = KEYR_KEY1;
  STM_FLASH->BANK[bank].KEYR = KEYR_KEY2;
}
/*----------------------------------------------------------------------------*/
static inline enum Result waitForCompletion(unsigned int bank)
{
  uint32_t sr;

  while ((sr = STM_FLASH->BANK[bank].SR) & SR_BSY);

  if (sr & SR_WRPRTERR)
    return E_ACCESS;
  if (sr & SR_PGERR)
    return E_VALUE;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashBaseConfig * const config = configBase;
  assert(config->bank < FLASH_BANK_END);

#ifndef CONFIG_PLATFORM_STM32_FLASH_BANK2
  assert(config->bank == FLASH_BANK_1);
#endif

  struct FlashBase * const interface = object;

  switch (IDCODE_DEV_ID_VALUE(STM_DBG->IDCODE))
  {
    case DEV_ID_HIGH_DENSITY:
    case DEV_ID_CONNECTIVITY:
    case DEV_ID_XL_DENSITY:
      interface->large = true;
      break;

    default:
      interface->large = false;
      break;
  }

  interface->size = STM_OB->FID * 1024;

  if (interface->size < FLASH_XL_DENSITY_MIN && config->bank != FLASH_BANK_1)
    return E_VALUE;

  interface->bank = config->bank;
  interface->parallelism = 0;
  interface->uniform = true;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum Result flashBaseErase([[maybe_unused]] struct FlashBase *interface,
    uint32_t position)
{
#ifdef CONFIG_PLATFORM_STM32_FLASH_BANK2
  const unsigned int bank = (unsigned int)interface->bank;
#else
  static const unsigned int bank = 0;
#endif

  unlockFlash(bank);
  STM_FLASH->BANK[bank].SR = SR_PGERR | SR_WRPRTERR | SR_EOP;
  STM_FLASH->BANK[bank].CR = CR_PER;
  STM_FLASH->BANK[bank].AR = position;
  STM_FLASH->BANK[bank].CR |= CR_STRT;

  const enum Result res = waitForCompletion(bank);

  STM_FLASH->BANK[bank].CR = 0;
  lockFlash(bank);

  return res;
}
/*----------------------------------------------------------------------------*/
uintptr_t flashBaseGetAddress(
    [[maybe_unused]] const struct FlashBase *interface)
{
#ifdef CONFIG_PLATFORM_STM32_FLASH_BANK2
  return interface->bank == FLASH_BANK_1 ?
      FLASH_BASE_ADDRESS : (FLASH_BASE_ADDRESS + FLASH_XL_DENSITY_MIN);
#else
  return FLASH_BASE_ADDRESS;
#endif
}
/*----------------------------------------------------------------------------*/
size_t flashBaseGetGeometry(const struct FlashBase *interface,
    struct FlashGeometry *geometry, size_t capacity)
{
  if (capacity)
  {
    if (interface->large)
    {
      geometry[0].count = interface->size / FLASH_PAGE_SIZE_LARGE;
      geometry[0].size = FLASH_PAGE_SIZE_LARGE;
      geometry[0].time = 40;
    }
    else
    {
      geometry[0].count = interface->size / FLASH_PAGE_SIZE_SMALL;
      geometry[0].size = FLASH_PAGE_SIZE_SMALL;
      geometry[0].time = 40;
    }

    return 1;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
enum Result flashBaseWrite(struct FlashBase *interface, uint32_t position,
    const void *buffer, size_t length)
{
#ifdef CONFIG_PLATFORM_STM32_FLASH_BANK2
  const unsigned int bank = (unsigned int)interface->bank;
#else
  static const unsigned int bank = 0;
#endif

  unlockFlash(bank);
  STM_FLASH->BANK[bank].SR = SR_PGERR | SR_WRPRTERR | SR_EOP;
  STM_FLASH->BANK[bank].CR = CR_PG;

  const uint8_t *input = buffer;
  uintptr_t output = flashBaseGetAddress(interface) + position;
  const uintptr_t end = output + length;
  enum Result res = E_OK;

  while (res == E_OK && output < end)
  {
    uint16_t word;

    memcpy(&word, input, sizeof(word));      
    *(uint16_t *)output = word;
    res = waitForCompletion(bank);

    input += sizeof(word);
    output += sizeof(word);
  }

  STM_FLASH->BANK[bank].CR = 0;
  lockFlash(bank);

  return res;
}
