/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/lpc/flash_base.h>
#include <halm/platform/lpc/flash_defs.h>
#include <halm/platform/lpc/iap.h>
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const FlashBase = &(const struct EntityClass){
    .size = sizeof(struct FlashBase),
    .init = flashInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct FlashBase * const interface = object;
  const uint32_t id = flashReadId();

  interface->bank = 0;
  interface->uniform = true;

  switch (id)
  {
    case CODE_LPC11E11_101:
      interface->size = 8 * 1024;
      break;

    case CODE_LPC11E12_201:
      interface->size = 16 * 1024;
      break;

    case CODE_LPC11E13_301:
      interface->size = 24 * 1024;
      break;

    case CODE_LPC11E14_401:
      interface->size = 32 * 1024;
      break;

    case CODE_LPC11E66:
    case CODE_LPC11U66:
      interface->size = 64 * 1024;
      break;

    case CODE_LPC11E36_501:
      interface->size = 96 * 1024;
      break;

    case CODE_LPC11E37_401:
    case CODE_LPC11E37_501:
      interface->size = 128 * 1024;
      break;

    case CODE_LPC11E67:
    case CODE_LPC11U67_1:
    case CODE_LPC11U67_2:
      interface->size = 128 * 1024;
      interface->uniform = false;
      break;

    case CODE_LPC11E68:
    case CODE_LPC11U68_1:
    case CODE_LPC11U68_2:
      interface->size = 256 * 1024;
      interface->uniform = false;
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
size_t flashBaseGetGeometry(const struct FlashBase *interface,
    struct FlashGeometry *geometry, size_t capacity)
{
  if (interface->uniform)
  {
    if (capacity < 1)
      return 0;

    geometry[0].count = interface->size / FLASH_SECTOR_SIZE_0;
    geometry[0].size = FLASH_SECTOR_SIZE_0;
    geometry[0].time = 100;

    return 1;
  }
  else
  {
    if (capacity < 2)
      return 0;

    geometry[0].count = FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0;
    geometry[0].size = FLASH_SECTOR_SIZE_0;
    geometry[0].time = 100;

    geometry[1].count =
        (interface->size - FLASH_SECTORS_BORDER) / FLASH_SECTOR_SIZE_1;
    geometry[1].size = FLASH_SECTOR_SIZE_1;
    geometry[1].time = 100;

    return 2;
  }
}
