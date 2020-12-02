/*
 * eeprom_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/lpc43xx/eeprom_base.h>
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EepromBase = &(const struct EntityClass){
    .size = sizeof(struct EepromBase),
    .init = eepromInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
extern const unsigned long _seeprom;
extern const unsigned long _eeeprom;
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct EepromBase * const interface = object;

  interface->address = (uint32_t)&_seeprom;
  interface->size = (uint32_t)&_eeeprom - (uint32_t)&_seeprom;
  return E_OK;
}
