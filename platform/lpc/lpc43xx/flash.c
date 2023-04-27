/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/lpc/flash.h>
#include <halm/platform/lpc/flash_defs.h>
#include <halm/platform/lpc/iap.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *, uint32_t);
static bool isSectorPositionValid(const struct Flash *, uint32_t);
static uint32_t positionToAddress(const struct Flash *, uint32_t);
static size_t totalFlashSize(size_t);
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *, const void *);
static enum Result flashGetParam(void *, int, void *);
static enum Result flashSetParam(void *, int, const void *);
static size_t flashRead(void *, void *, size_t);
static size_t flashWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Flash = &(const struct InterfaceClass){
    .size = sizeof(struct Flash),
    .init = flashInit,
    .deinit = NULL, /* Default destructor */

    .setCallback = NULL,
    .getParam = flashGetParam,
    .setParam = flashSetParam,
    .read = flashRead,
    .write = flashWrite
};
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= totalFlashSize(interface->base.size))
    return false;
  if (position & (FLASH_PAGE_SIZE - 1))
    return false;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool isSectorPositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= totalFlashSize(interface->base.size))
    return false;

  const uint32_t address =
      positionToAddress(interface, position) & FLASH_BANK_MASK;
  const uint32_t mask = address < FLASH_SECTORS_BORDER ?
      (FLASH_SECTOR_SIZE_0 - 1) : (FLASH_SECTOR_SIZE_1 - 1);

  return !(address & mask);
}
/*----------------------------------------------------------------------------*/
static uint32_t positionToAddress(const struct Flash *interface,
    uint32_t position)
{
  if (position < FLASH_SIZE_DECODE_A(interface->base.size))
  {
    return FLASH_BANK_A + position;
  }
  else
  {
    return FLASH_BANK_B
        + (position - FLASH_SIZE_DECODE_A(interface->base.size));
  }
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
  const enum Result res = FlashBase->init(interface, NULL);

  if (res == E_OK)
    interface->position = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result flashGetParam(void *object, int parameter, void *data)
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

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
      *(uint32_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(uint32_t *)data = totalFlashSize(interface->base.size);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result flashSetParam(void *object, int parameter, const void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash options */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (!isPagePositionValid(interface, position))
        return E_ADDRESS;

      flashInitWrite();
      return flashErasePage(positionToAddress(interface, position));
    }

    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (!isSectorPositionValid(interface, position))
        return E_ADDRESS;
      if (flashBlankCheckSector(positionToAddress(interface, position)) == E_OK)
        return E_OK;

      flashInitWrite();
      return flashEraseSector(positionToAddress(interface, position));
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position < totalFlashSize(interface->base.size))
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

  if (interface->position + length <= totalFlashSize(interface->base.size))
  {
    const uint32_t address = positionToAddress(interface, interface->position);

    memcpy(buffer, (const void *)address, length);
    interface->position += (uint32_t)length;
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
  {
    interface->position += (uint32_t)length;
    return length;
  }
  else
    return 0;
}
