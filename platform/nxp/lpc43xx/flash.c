/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <string.h>
#include <halm/platform/nxp/flash.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc43xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *, size_t);
static bool isSectorPositionValid(const struct Flash *, size_t);
static uint32_t positionToAddress(const struct Flash *, size_t);
static size_t totalFlashSize(size_t);
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *, const void *);
static enum Result flashSetCallback(void *, void (*)(void *), void *);
static enum Result flashGetParam(void *, enum IfParameter, void *);
static enum Result flashSetParam(void *, enum IfParameter, const void *);
static size_t flashRead(void *, void *, size_t);
static size_t flashWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass flashTable = {
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
const struct InterfaceClass * const Flash = &flashTable;
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *interface, size_t position)
{
  if (position >= totalFlashSize(interface->size))
    return false;
  if (position & (FLASH_PAGE_SIZE - 1))
    return false;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool isSectorPositionValid(const struct Flash *interface,
    size_t position)
{
  if (position >= totalFlashSize(interface->size))
    return false;

  const uint32_t address = positionToAddress(interface, position)
      & FLASH_BANK_MASK;
  const uint32_t mask = address < FLASH_SECTORS_BORDER ?
      (FLASH_SECTOR_SIZE_0 - 1) : (FLASH_SECTOR_SIZE_1 - 1);

  return !(address & mask);
}
/*----------------------------------------------------------------------------*/
static uint32_t positionToAddress(const struct Flash *interface,
    size_t position)
{
  if (position < FLASH_SIZE_DECODE_A(interface->size))
    return FLASH_BANK_A + position;
  else
    return FLASH_BANK_B + (position - FLASH_SIZE_DECODE_A(interface->size));
}
/*----------------------------------------------------------------------------*/
static size_t totalFlashSize(size_t size)
{
  return FLASH_SIZE_DECODE_A(size) + FLASH_SIZE_DECODE_B(size);
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;
  const uint32_t id = flashReadId();

  if (!(id & FLASH_AVAILABLE))
    return E_ERROR; /* Flashless part */

  const uint32_t memory = flashReadConfigId() & FLASH_SIZE_MASK;

  switch (memory)
  {
    case FLASH_SIZE_256_256:
      interface->size = FLASH_SIZE_ENCODE(256 * 1024, 256 * 1024);
      break;

    case FLASH_SIZE_384_384:
      interface->size = FLASH_SIZE_ENCODE(384 * 1024, 384 * 1024);
      break;

    case FLASH_SIZE_512_0:
      interface->size = FLASH_SIZE_ENCODE(512 * 1024, 0);
      break;

    case FLASH_SIZE_512_512:
      interface->size = FLASH_SIZE_ENCODE(512 * 1024, 512 * 1024);
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

  switch (parameter)
  {
    case IF_POSITION:
      *(size_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(size_t *)data = totalFlashSize(interface->size);
      return E_OK;

    case IF_FLASH_PAGE_SIZE:
      *(size_t *)data = FLASH_PAGE_SIZE;
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

  /* Additional Flash options */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const size_t position = *(const size_t *)data;

      if (!isSectorPositionValid(interface, position))
        return E_ADDRESS;
      if (flashBlankCheckSector(position) == E_OK)
        return E_OK;

      flashInitWrite();
      return flashEraseSector(positionToAddress(interface, position));
    }

    case IF_FLASH_ERASE_PAGE:
    {
      const size_t position = *(const size_t *)data;

      if (!isPagePositionValid(interface, position))
        return E_ADDRESS;

      flashInitWrite();
      return flashErasePage(positionToAddress(interface, position));
    }

    default:
      break;
  }

  switch (parameter)
  {
    case IF_POSITION:
    {
      const size_t position = *(const size_t *)data;

      if (position < totalFlashSize(interface->size))
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

  if (interface->position + length <= totalFlashSize(interface->size))
  {
    const uint32_t address = positionToAddress(interface, interface->position);

    memcpy(buffer, (const void *)address, length);
    return length;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static size_t flashWrite(void *object, const void *buffer, size_t length)
{
  struct Flash * const interface = object;
  const uint32_t address = positionToAddress(interface, interface->position);

  flashInitWrite();

  if (flashWriteBuffer(address, buffer, length) == E_OK)
    return length;
  else
    return 0;
}
