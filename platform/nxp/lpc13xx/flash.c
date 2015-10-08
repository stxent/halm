/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <platform/nxp/flash.h>
#include <platform/nxp/lpc13xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isPageAddressValid(const struct Flash *, uint32_t);
static inline bool isSectorAddressValid(const struct Flash *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *, const void *);
static void flashDeinit(void *);
static enum result flashCallback(void *, void (*)(void *), void *);
static enum result flashGet(void *, enum ifOption, void *);
static enum result flashSet(void *, enum ifOption, const void *);
static uint32_t flashRead(void *, uint8_t *, uint32_t);
static uint32_t flashWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass flashTable = {
    .size = sizeof(struct Flash),
    .init = flashInit,
    .deinit = flashDeinit,

    .callback = flashCallback,
    .get = flashGet,
    .set = flashSet,
    .read = flashRead,
    .write = flashWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Flash = &flashTable;
/*----------------------------------------------------------------------------*/
static inline bool isPageAddressValid(const struct Flash *interface,
    uint32_t address)
{
  return !(address & (FLASH_PAGE_SIZE - 1)) && address < interface->size;
}
/*----------------------------------------------------------------------------*/
static inline bool isSectorAddressValid(const struct Flash *interface,
    uint32_t address)
{
  return !(address & (FLASH_SECTOR_SIZE - 1)) && address < interface->size;
}
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;

  const uint32_t id = flashReadId();

  switch (id)
  {
    case CODE_LPC1311:
    case CODE_LPC1311_01:
      interface->size = 8 * 1024;
      break;

    case CODE_LPC1342:
      interface->size = 16 * 1024;
      break;

    case CODE_LPC1313:
    case CODE_LPC1313_01:
    case CODE_LPC1343:
      interface->size = 32 * 1024;
      break;

    default:
      return E_ERROR;
  }

  interface->callback = 0;
  interface->position = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void flashDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result flashCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Flash * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result flashGet(void *object, enum ifOption option, void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash options */
  switch ((enum flashOption)option)
  {
    case IF_FLASH_SECTOR_SIZE:
      /* Fixed sector size. */
      *(uint32_t *)data = FLASH_SECTOR_SIZE;
      return E_OK;

    default:
      break;
  }

  switch (option)
  {
    case IF_ADDRESS:
      *(uint32_t *)data = interface->position;
      return E_OK;

    case IF_ALIGNMENT:
      *(uint32_t *)data = FLASH_PAGE_SIZE;
      return E_OK;

    case IF_STATUS:
      return E_OK;

    case IF_SIZE:
      *((uint32_t *)data) = interface->size;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result flashSet(void *object, enum ifOption option,
    const void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash options */
  switch ((enum flashOption)option)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t address = *(const uint32_t *)data;

      if (!isSectorAddressValid(interface, address))
        return E_VALUE;

      if (flashBlankCheckSector(address) == E_OK)
        return E_OK;

      return flashEraseSector(address);
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_ADDRESS:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (!isPageAddressValid(interface, position))
        return E_VALUE;

      interface->position = position;
      return E_OK;
    }

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t flashRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Flash * const interface = object;

  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  memcpy(buffer, (const void *)interface->position, length);

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t flashWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Flash * const interface = object;

  if (!isPageAddressValid(interface, interface->position))
    return 0;
  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  if (flashWriteBuffer(interface->position, buffer, length) != E_OK)
    return 0;

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  return length;
}
