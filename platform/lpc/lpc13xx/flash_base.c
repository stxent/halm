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
static enum Result flashInit(void *object, const void *)
{
  struct FlashBase * const interface = object;
  const uint32_t id = flashReadId();

  switch (id)
  {
    case CODE_LPC1311:
    case CODE_LPC1311_01:
      interface->size = 8 * 1024;
      break;

    case CODE_LPC1342:
      interface->size = 16 * 1024;
      break;

    case CODE_LPC1313:
    case CODE_LPC1313_01:
    case CODE_LPC1343:
      interface->size = 32 * 1024;
      break;

    default:
      return E_ERROR;
  }

  interface->bank = 0;
  interface->uniform = true;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
size_t flashBaseGetGeometry(const struct FlashBase *interface,
    struct FlashGeometry *geometry, size_t capacity)
{
  if (capacity)
  {
    geometry[0].count = interface->size / FLASH_SECTOR_SIZE;
    geometry[0].size = FLASH_SECTOR_SIZE;
    geometry[0].time = 100;

    return 1;
  }

  return 0;
}
