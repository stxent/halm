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
    [[maybe_unused]] const void *configBase)
{
  struct FlashBase * const interface = object;
  const uint32_t id = flashReadId();

  if (IS_LPC1110(id))
    interface->size = 4 * 1024;
  else if (IS_LPC1111(id))
    interface->size = 8 * 1024;
  else if (IS_LPC1112(id) || IS_LPC11CX2(id))
    interface->size = 16 * 1024;
  else if (IS_LPC1113(id))
    interface->size = 24 * 1024;
  else if (IS_LPC1114(id) || IS_LPC11CX4(id))
    interface->size = 32 * 1024;
  else if (id == CODE_LPC1114_323)
    interface->size = 48 * 1024;
  else if (id == CODE_LPC1114_333)
    interface->size = 56 * 1024;
  else if (IS_LPC1115(id))
    interface->size = 64 * 1024;
  else
    return E_ERROR;

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
