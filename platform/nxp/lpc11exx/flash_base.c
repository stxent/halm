/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/flash_base.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc11exx/flash_defs.h>
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
    case CODE_LPC11E67:
    case CODE_LPC11U67_1:
    case CODE_LPC11U67_2:
      interface->size = 128 * 1024;
      break;

    case CODE_LPC11E68:
    case CODE_LPC11U68_1:
    case CODE_LPC11U68_2:
      interface->size = 256 * 1024;
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}
