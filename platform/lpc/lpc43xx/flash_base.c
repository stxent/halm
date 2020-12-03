/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/flash_base.h>
#include <halm/platform/lpc/iap.h>
#include <halm/platform/lpc/lpc43xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const FlashBase = &(const struct EntityClass){
    .size = sizeof(struct FlashBase),
    .init = flashInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct FlashBase * const interface = object;
  const uint32_t id = flashReadId();

  if (!(id & FLASH_AVAILABLE))
    return E_ERROR; /* Flashless part */

  const uint32_t memory = flashReadConfigId() & FLASH_SIZE_MASK;

  switch (memory)
  {
    case FLASH_SIZE_256_256:
      interface->size = FLASH_SIZE_ENCODE(256 * 1024, 256 * 1024);
      break;

    case FLASH_SIZE_384_384:
      interface->size = FLASH_SIZE_ENCODE(384 * 1024, 384 * 1024);
      break;

    case FLASH_SIZE_512_0:
      interface->size = FLASH_SIZE_ENCODE(512 * 1024, 0);
      break;

    case FLASH_SIZE_512_512:
      interface->size = FLASH_SIZE_ENCODE(512 * 1024, 512 * 1024);
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}
