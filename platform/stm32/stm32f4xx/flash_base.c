/*
 * flash_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/stm32/flash_base.h>
#include <halm/platform/stm32/flash_defs.h>
#include <halm/platform/platform_defs.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static inline void lockFlash(void);
static inline void unlockFlash(void);
static inline enum Result waitForCompletion(void);

static enum Result flashInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const FlashBase = &(const struct EntityClass){
    .size = sizeof(struct FlashBase),
    .init = flashInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static inline void lockFlash(void)
{
  STM_FLASH->CR |= CR_LOCK;
}
/*----------------------------------------------------------------------------*/
static inline void unlockFlash(void)
{
	STM_FLASH->KEYR = KEYR_KEY1;
	STM_FLASH->KEYR = KEYR_KEY2;
}
/*----------------------------------------------------------------------------*/
static inline enum Result waitForCompletion(void)
{
  uint32_t sr;

  while ((sr = STM_FLASH->SR) & SR_BSY);

  if (sr & (SR_WRPERR))
    return E_ACCESS;
  if (sr & (SR_PGAERR))
    return E_ADDRESS;
  if (sr & (SR_PGPERR | SR_PGSERR))
    return E_INTERFACE;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashBaseConfig * const config = configBase;
  struct FlashBase * const interface = object;

  // TODO Add dual-bank memory support for STM32F4
  if (config->bank != FLASH_BANK_1)
    return E_VALUE;

  /* 
   * Parallelism configuration depends on IC voltage:
   *   2.7 - 3.6 with external VPP: x64
   *   2.7 - 3.6: x32
   *   2.1 - 2.7: x16
   *   1.7 - 2.1: x8
   */
  switch (config->voltage)
  {
    case VR_2V7_3V6_VPP:
      interface->parallelism = PSIZE_X64;
      break;

    case VR_2V7_3V6:
      interface->parallelism = PSIZE_X32;
      break;

    case VR_2V4_2V7:
    case VR_2V1_2V4:
      interface->parallelism = PSIZE_X16;
      break;

    default:
      interface->parallelism = PSIZE_X8;
      break;
  }

  interface->size = STM_OB->FID * 1024;
  interface->bank = config->bank;
  interface->large = false;
  interface->uniform = false;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum Result flashBaseErase(struct FlashBase *interface, uint32_t position)
{
  const unsigned int sector = addressToSector(position);

  unlockFlash();
  STM_FLASH->SR = SR_WRPERR | SR_PGAERR | SR_PGPERR | SR_PGSERR;
  STM_FLASH->CR = CR_SER | CR_SNB(sector) | CR_PSIZE(interface->parallelism);
  STM_FLASH->CR |= CR_STRT;

  const enum Result res = waitForCompletion();

  STM_FLASH->CR = 0;
  lockFlash();

  return res;
}
/*----------------------------------------------------------------------------*/
uintptr_t flashBaseGetAddress(const struct FlashBase *)
{
  return FLASH_BASE_ADDRESS;
}
/*----------------------------------------------------------------------------*/
size_t flashBaseGetGeometry(const struct FlashBase *interface,
    struct FlashGeometry *geometry, size_t capacity)
{
  if (capacity >= 3)
  {
    geometry[0].count = FLASH_SECTORS_COUNT_0;
    geometry[0].size = FLASH_SECTOR_SIZE_0;
    geometry[0].time = 800; /* Milliseconds, worst-case value */

    geometry[1].count = FLASH_SECTORS_COUNT_1;
    geometry[1].size = FLASH_SECTOR_SIZE_1;
    geometry[1].time = 2400; /* Milliseconds, worst-case value */

    geometry[2].count =
        (interface->size - FLASH_SECTORS_BORDER_1) / FLASH_SECTOR_SIZE_2;
    geometry[2].size = FLASH_SECTOR_SIZE_2;
    geometry[2].time = 4000; /* Milliseconds, worst-case value */

    return 3;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
enum Result flashBaseWrite(struct FlashBase *interface, uint32_t position,
    const void *buffer, size_t length)
{
  unlockFlash();
  STM_FLASH->SR = SR_WRPERR | SR_PGAERR | SR_PGPERR | SR_PGSERR;
  STM_FLASH->CR = CR_PG | CR_PSIZE(interface->parallelism);

  const uint8_t *input = buffer;
  uintptr_t output = flashBaseGetAddress(interface) + position;
  const uintptr_t end = output + length;
  enum Result res = E_OK;

  if (interface->parallelism == PSIZE_X32)
  {
    while (res == E_OK && output < end)
    {
      uint32_t word;

      memcpy(&word, input, sizeof(word));      
      *(uint32_t *)output = word;
      res = waitForCompletion();

      input += sizeof(word);
      output += sizeof(word);
    }
  }
  else if (interface->parallelism == PSIZE_X16)
  {
    while (res == E_OK && output < end)
    {
      uint16_t word;

      memcpy(&word, input, sizeof(word));      
      *(uint16_t *)output = word;
      res = waitForCompletion();

      input += sizeof(word);
      output += sizeof(word);
    }
  }
  else
  {
    while (res == E_OK && output < end)
    {
      *(uint8_t *)output = *input;
      res = waitForCompletion();

      ++input;
      ++output;
    }
  }

  STM_FLASH->CR = 0;
  lockFlash();

  return res;
}
