/*
 * flash.c
 * Copyright (C) 2015, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/generic/flash.h>
#include <halm/platform/lpc/gen_1/flash.h>
#include <halm/platform/lpc/gen_1/flash_defs.h>
#include <halm/platform/lpc/iap.h>
#include <string.h>
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
    .deinit = 0, /* Default destructor */

    .setCallback = 0,
    .getParam = flashGetParam,
    .setParam = flashSetParam,
    .read = flashRead,
    .write = flashWrite
};
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;
  const enum Result res = FlashBase->init(interface, 0);

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
      *(uint32_t *)data = interface->base.size;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result flashSetParam(void *object, int parameter, const void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash parameters */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isPagePositionValid(position))
        return E_ADDRESS;

      return flashErasePage(position);
    }

    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isSectorPositionValid(position))
        return E_ADDRESS;

      if (flashBlankCheckSector(position) == E_OK)
        return E_OK;

      return flashEraseSector(position);
    }

    default:
      break;
  }

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
static size_t flashRead(void *object, void *buffer, size_t length)
{
  struct Flash * const interface = object;

  if (interface->position + length <= interface->base.size)
  {
    memcpy(buffer, (const void *)interface->position, length);
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

  if (flashWriteBuffer(interface->position, buffer, length) == E_OK)
  {
    interface->position += (uint32_t)length;
    return length;
  }
  else
    return 0;
}
