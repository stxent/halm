/*
 * eeprom.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <string.h>
#include <halm/platform/nxp/eeprom.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc11exx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Eeprom *, uintptr_t);
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
static enum Result eepromGetParam(void *, enum IfParameter, void *);
static enum Result eepromSetParam(void *, enum IfParameter, const void *);
static size_t eepromRead(void *, void *, size_t);
static size_t eepromWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Eeprom = &(const struct InterfaceClass){
    .size = sizeof(struct Eeprom),
    .init = eepromInit,
    .deinit = 0, /* Default destructor */

    .setCallback = 0,
    .getParam = eepromGetParam,
    .setParam = eepromSetParam,
    .read = eepromRead,
    .write = eepromWrite
};
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Eeprom *interface,
    uintptr_t address)
{
  return address < interface->size;
}
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Eeprom * const interface = object;
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

  interface->position = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result eepromGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Eeprom * const interface = object;

  switch (parameter)
  {
    case IF_POSITION:
      *(uint32_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(uint32_t *)data = interface->size;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result eepromSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Eeprom * const interface = object;

  switch (parameter)
  {
    case IF_POSITION:
    {
      const uintptr_t position = *(const uint32_t *)data;

      if (isAddressValid(interface, position))
      {
        interface->position = position;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t eepromRead(void *object, void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  if (eepromReadBuffer(interface->position, buffer, length) == E_OK)
    return length;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static size_t eepromWrite(void *object, const void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  if (eepromWriteBuffer(interface->position, buffer, length) == E_OK)
    return length;
  else
    return 0;
}
