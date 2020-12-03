/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/flash_base.h>
#include <halm/platform/lpc/iap.h>
#include <halm/platform/lpc/lpc13uxx/flash_defs.h>
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

  switch (id)
  {
    case CODE_LPC1315:
    case CODE_LPC1345:
      interface->size = 32 * 1024;
      break;

    case CODE_LPC1316:
    case CODE_LPC1346:
      interface->size = 48 * 1024;
      break;

    case CODE_LPC1317:
    case CODE_LPC1347:
      interface->size = 64 * 1024;
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}
