/*
 * flash.c
 * Copyright (C) 2015, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/lpc/flash.h>
#include <halm/platform/lpc/flash_defs.h>
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
    .deinit = NULL, /* Default destructor */

    .setCallback = NULL,
    .getParam = flashGetParam,
    .setParam = flashSetParam,
    .read = flashRead,
    .write = flashWrite
};
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *)
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
    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isSectorPositionValid(position, interface->base.uniform))
        return E_ADDRESS;

      if (flashBlankCheckSector(position, interface->base.uniform) == E_OK)
        return E_OK;

      return flashEraseSector(position, interface->base.uniform);
    }

    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isPagePositionValid(position))
        return E_ADDRESS;

      return flashErasePage(position, interface->base.uniform);
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
  /* Buffer length should be aligned on the page boundary */
  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  struct Flash * const interface = object;

  /* Address should be aligned on the page boundary */
  if (interface->position & (FLASH_PAGE_SIZE - 1))
    return 0;
  /* Address should be inside the boundary */
  if (interface->position + length > interface->base.size)
    return 0;

  size_t left = length;
  const uint8_t *input = buffer;
  uint32_t address = interface->position;

  while (left)
  {
    const size_t chunk = MIN(left, FLASH_PAGE_SIZE);

    flashInitWrite();

    if (flashWriteBuffer(address, interface->base.uniform,
        input, chunk) != E_OK)
    {
      break;
    }

    left -= chunk;
    input += chunk;
    address += (uint32_t)chunk;
  }

  interface->position += (uint32_t)(length - left);
  return length - left;
}
/*----------------------------------------------------------------------------*/
void *flashGetAddress(const void *)
{
  return (void *)0;
}
/*----------------------------------------------------------------------------*/
size_t flashGetGeometry(const void *object, struct FlashGeometry *geometry,
    size_t capacity)
{
  return flashBaseGetGeometry(object, geometry, capacity);
}
