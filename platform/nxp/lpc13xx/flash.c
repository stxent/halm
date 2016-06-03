/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <halm/platform/nxp/flash.h>
#include <halm/platform/nxp/lpc13xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isPageAddressValid(const struct Flash *, uint32_t);
static inline bool isSectorAddressValid(const struct Flash *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *, const void *);
static void flashDeinit(void *);
static enum result flashCallback(void *, void (*)(void *), void *);
static enum result flashGet(void *, enum ifOption, void *);
static enum result flashSet(void *, enum ifOption, const void *);
static size_t flashRead(void *, void *, size_t);
static size_t flashWrite(void *, const void *, size_t);
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

  interface->position = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void flashDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result flashCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result flashGet(void *object, enum ifOption option, void *data)
{
  struct Flash * const interface = object;

  switch (option)
  {
    case IF_POSITION:
      *(size_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(size_t *)data = interface->size;
      return E_OK;

    default:
      return E_INVALID;
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
      const size_t address = *(const size_t *)data;

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
    case IF_POSITION:
    {
      const size_t position = *(const size_t *)data;

      if (!isPageAddressValid(interface, position))
        return E_VALUE;

      interface->position = position;
      return E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t flashRead(void *object, void *buffer, size_t length)
{
  struct Flash * const interface = object;

  if (!isPageAddressValid(interface, interface->position))
    return 0;
  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  memcpy(buffer, (const void *)interface->position, length);

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t flashWrite(void *object, const void *buffer, size_t length)
{
  struct Flash * const interface = object;

  if (!isPageAddressValid(interface, interface->position))
    return 0;
  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  if (flashWriteBuffer(interface->position, buffer, length) != E_OK)
    return 0;

  return length;
}
