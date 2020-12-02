/*
 * eeprom_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/eeprom_base.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc11exx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EepromBase = &(const struct EntityClass){
    .size = sizeof(struct EepromBase),
    .init = eepromInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct EepromBase * const interface = object;
  const uint32_t id = flashReadId();

  switch (id)
  {
    case CODE_LPC11E11_101:
      interface->size = 512;
      break;

    case CODE_LPC11E12_201:
      interface->size = 1024;
      break;

    case CODE_LPC11E13_301:
      interface->size = 2 * 1024;
      break;

    case CODE_LPC11E14_401:
    case CODE_LPC11E36_501:
    case CODE_LPC11E37_401:
    case CODE_LPC11E37_501:
    case CODE_LPC11E66:
    case CODE_LPC11E67:
    case CODE_LPC11E68:
    case CODE_LPC11U66:
    case CODE_LPC11U67_1:
    case CODE_LPC11U67_2:
    case CODE_LPC11U68_1:
    case CODE_LPC11U68_2:
      interface->size = 4 * 1024 - 64;
      break;

    default:
      return E_ERROR;
  }

  return E_OK;
}