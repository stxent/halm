/*
 * flash.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <string.h>
#include <halm/platform/nxp/flash.h>
#include <halm/platform/nxp/iap.h>
#include <halm/platform/nxp/lpc11xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline bool isSectorPositionValid(const struct Flash *, size_t);
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
static inline bool isSectorPositionValid(const struct Flash *interface,
    size_t position)
{
  return !(position & (FLASH_SECTOR_SIZE - 1)) && position < interface->size;
}
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;
  const uint32_t id = flashReadId();

  if (IS_LPC1110(id))
    interface->size = 4 * 1024;
  else if (IS_LPC1111(id))
    interface->size = 8 * 1024;
  else if (IS_LPC1112(id) || IS_LPC11CX2(id))
    interface->size = 16 * 1024;
  else if (IS_LPC1113(id))
    interface->size = 24 * 1024;
  else if (IS_LPC1114(id) || IS_LPC11CX4(id))
    interface->size = 32 * 1024;
  else if (id == CODE_LPC1114_323)
    interface->size = 48 * 1024;
  else if (id == CODE_LPC1114_333)
    interface->size = 56 * 1024;
  else if (IS_LPC1115(id))
    interface->size = 64 * 1024;
  else
    return E_ERROR;

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

    case IF_FLASH_PAGE_SIZE:
      *(size_t *)data = FLASH_PAGE_SIZE;
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
  switch ((enum FlashOption)option)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const size_t position = *(const size_t *)data;

      if (!isSectorPositionValid(interface, position))
        return E_ADDRESS;

      if (flashBlankCheckSector(position) == E_OK)
        return E_OK;

      return flashEraseSector(position);
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_POSITION:
    {
      const size_t position = *(const size_t *)data;

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
