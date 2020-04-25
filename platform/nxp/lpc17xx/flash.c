/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <string.h>
#include <halm/generic/flash.h>
#include <halm/platform/nxp/flash.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc17xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(const struct Flash *, size_t);
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *, const void *);
static enum Result flashSetCallback(void *, void (*)(void *), void *);
static enum Result flashGetParam(void *, enum IfParameter, void *);
static enum Result flashSetParam(void *, enum IfParameter, const void *);
static size_t flashRead(void *, void *, size_t);
static size_t flashWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Flash = &(const struct InterfaceClass){
    .size = sizeof(struct Flash),
    .init = flashInit,
    .deinit = 0, /* Default destructor */

    .setCallback = flashSetCallback,
    .getParam = flashGetParam,
    .setParam = flashSetParam,
    .read = flashRead,
    .write = flashWrite
};
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(const struct Flash *interface,
    size_t position)
{
  if (position >= interface->size)
    return false;

  if (position < FLASH_SECTORS_BORDER)
    return !(position & (FLASH_SECTOR_SIZE_0 - 1));
  else
    return !(position & (FLASH_SECTOR_SIZE_1 - 1));
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;
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

  interface->position = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result flashSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result flashGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash parameters */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_PAGE_SIZE:
      *(uint32_t *)data = FLASH_PAGE_SIZE;
      return E_OK;

    default:
      break;
  }

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
static enum Result flashSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash parameters */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const uintptr_t position = *(const uint32_t *)data;

      if (!isSectorPositionValid(interface, position))
        return E_ADDRESS;

      if (flashBlankCheckSector(position) == E_OK)
        return E_OK;

      return flashEraseSector(position);
    }

    default:
      break;
  }

  switch (parameter)
  {
    case IF_POSITION:
    {
      const uintptr_t position = *(const uint32_t *)data;

      if (position < interface->size)
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

  if (interface->position + length <= interface->size)
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
