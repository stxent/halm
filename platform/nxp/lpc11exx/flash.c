/*
 * flash.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <halm/platform/nxp/flash.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc11exx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Flash *, uint32_t, size_t);
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
static inline bool isAddressValid(const struct Flash *interface,
    uint32_t address, size_t length)
{
  return address < interface->size && address + length <= interface->size;
}
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
  if (address >= interface->size)
    return false;

  if (address < FLASH_SECTORS_BORDER)
    return !(address & (FLASH_SECTOR_SIZE_0 - 1));
  else
    return !(address & (FLASH_SECTOR_SIZE_1 - 1));
}
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;
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
        return E_ADDRESS;

      if (flashBlankCheckSector(address) == E_OK)
        return E_OK;

      return flashEraseSector(address);
    }

    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t address = *(const uint32_t *)data;

      if (!isPageAddressValid(interface, address))
        return E_ADDRESS;

      return flashErasePage(address);
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (isPageAddressValid(interface, position))
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
static size_t flashRead(void *object, void *buffer, size_t length)
{
  struct Flash * const interface = object;

  if (isAddressValid(interface, interface->position, length))
  {
    memcpy(buffer, (const void *)interface->position, length);
    return length;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static size_t flashWrite(void *object, const void *buffer, size_t length)
{
  struct Flash * const interface = object;

  if (flashWriteBuffer(interface->position, buffer, length) == E_OK)
    return length;
  else
    return 0;
}
