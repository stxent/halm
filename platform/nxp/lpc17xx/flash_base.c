/*
 * flash_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/flash_base.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc17xx/flash_defs.h>
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
    case CODE_LPC1751_00:
    case CODE_LPC1751:
      interface->size = 32 * 1024;
      break;

    case CODE_LPC1752:
      interface->size = 64 * 1024;
      break;

    case CODE_LPC1754:
    case CODE_LPC1764:
      interface->size = 128 * 1024;
      break;

    case CODE_LPC1756:
    case CODE_LPC1763:
    case CODE_LPC1765:
    case CODE_LPC1766:
      interface->size = 256 * 1024;
      break;

    case CODE_LPC1758:
    case CODE_LPC1759:
    case CODE_LPC1767:
    case CODE_LPC1768:
    case CODE_LPC1769:
      interface->size = 512 * 1024;
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}
