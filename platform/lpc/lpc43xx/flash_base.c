/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

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
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashBaseConfig * const config = configBase;
  struct FlashBase * const interface = object;
  const uint32_t id = flashReadId();

  if (!(id & FLASH_AVAILABLE))
    return E_ERROR; /* Flashless part */

  const uint32_t memory = flashReadConfigId() & FLASH_SIZE_MASK;

  switch (memory)
  {
    case FLASH_SIZE_256_256:
      interface->size = 256 * 1024;
      break;

    case FLASH_SIZE_384_384:
      interface->size = 384 * 1024;
      break;

    case FLASH_SIZE_512_0:
      if (config->bank)
        return E_VALUE;

      interface->size = 512 * 1024;
      break;

    case FLASH_SIZE_512_512:
      interface->size = 512 * 1024;
      break;

    default:
      return E_ERROR;
  }

  /* Bank 0 is Flash A, bank 1 is Flash B */
  interface->bank = config->bank;
  interface->uniform = false;

  return E_OK;
}
