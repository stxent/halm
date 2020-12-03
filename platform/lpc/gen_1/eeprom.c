/*
 * eeprom.c
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/eeprom.h>
#include <halm/platform/lpc/iap.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
static enum Result eepromGetParam(void *, int, void *);
static enum Result eepromSetParam(void *, int, const void *);
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
static enum Result eepromInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Eeprom * const interface = object;
  const enum Result res = EepromBase->init(interface, 0);

  if (res == E_OK)
    interface->position = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result eepromGetParam(void *object, int parameter, void *data)
{
  struct Eeprom * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
      *(uint32_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(uint32_t *)data = interface->base.size;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result eepromSetParam(void *object, int parameter, const void *data)
{
  struct Eeprom * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position < interface->base.size)
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
