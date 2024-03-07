/*
 * eeprom_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/lpc43xx/eeprom_base.h>
#include <halm/platform/lpc/system.h>
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
static void eepromDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EepromBase = &(const struct EntityClass){
    .size = sizeof(struct EepromBase),
    .init = eepromInit,
    .deinit = eepromDeinit
};
/*----------------------------------------------------------------------------*/
extern const unsigned long _seeprom;
extern const unsigned long _eeeprom;
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object, const void *)
{
  struct EepromBase * const interface = object;

  /* Enable clock to register interface and peripheral */
  sysClockEnable(CLK_M4_EEPROM);
  /* Reset registers to default values */
  sysResetEnable(RST_EEPROM);

  interface->address = (uint32_t)&_seeprom;
  interface->size = (uint32_t)&_eeeprom - (uint32_t)&_seeprom;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void eepromDeinit(void *)
{
  sysClockDisable(CLK_M4_EEPROM);
}
