/*
 * flash.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/stm32/flash.h>
#include <halm/platform/stm32/flash_defs.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static bool blankCheckMemory(const struct Flash *, uint32_t, uint32_t);

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
static bool blankCheckMemory(const struct Flash *interface, uint32_t position,
    uint32_t count)
{
  const uint32_t *start = (const uint32_t *)(
      flashBaseGetAddress(&interface->base) + (position & ~count));
  const uint32_t * const end = start + count / sizeof(uint32_t);

  while (start < end)
  {
    if (*start++ != 0xFFFFFFFFUL)
      return false;
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashBaseConfig * const config = configBase;
  struct Flash * const interface = object;

  const struct FlashBaseConfig baseConfig = {
      .bank = config != NULL ? config->bank : FLASH_BANK_1,
      .voltage = config != NULL ? config->voltage : VR_DEFAULT
  };
  const enum Result res = FlashBase->init(interface, &baseConfig);

  if (res == E_OK)
    interface->position = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result flashGetParam(void *object, int parameter, void *data)
{
  struct Flash * const interface = object;

  /* Additional flash memory parameters */
  switch ((enum FlashParameter)parameter)
  {
#ifdef FLASH_USE_SECTORS
    case IF_FLASH_SECTOR_SIZE:
      if (!interface->base.uniform)
      {
        /* Sector size is ambiguous, use geometry to calculate actual size */
        *(uint32_t *)data = 0;
      }
      else
        *(uint32_t *)data = addressToSectorSize(0);

      return E_OK;
#endif

#ifdef FLASH_USE_PAGES
    case IF_FLASH_SECTOR_SIZE:
      *(uint32_t *)data = addressToPageSize(interface->base.large);
      return E_OK;
#endif

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

  /* Additional flash memory parameters */
  switch ((enum FlashParameter)parameter)
  {
#ifdef FLASH_USE_SECTORS
    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isSectorPositionValid(position))
        return E_ADDRESS;

      const uint32_t count = addressToSectorSize(position);

      if (blankCheckMemory(interface, position, count))
        return E_OK;

      return flashBaseErase(&interface->base, position);
    }
#endif

#ifdef FLASH_USE_PAGES
    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (position >= interface->base.size)
        return E_ADDRESS;
      if (!isPagePositionValid(interface->base.large, position))
        return E_ADDRESS;

      const uint32_t count = addressToPageSize(interface->base.large);

      if (blankCheckMemory(interface, position, count))
        return E_OK;

      return flashBaseErase(&interface->base, position);
    }
#endif

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
    const uintptr_t start = flashBaseGetAddress(&interface->base);

    memcpy(buffer, (const void *)(start + interface->position), length);
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

  /* Address should be aligned on the word boundary */
  if ((interface->position & 0x03) != 0)
    return 0;
  /* Address should be inside the boundary */
  if (interface->position + length > interface->base.size)
    return 0;

  const enum Result res = flashBaseWrite(&interface->base, interface->position,
      buffer, length);

  if (res == E_OK)
  {
    interface->position += (uint32_t)length;
    return length;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
void *flashGetAddress(const void *object)
{
  return (void *)flashBaseGetAddress(object);
}
/*----------------------------------------------------------------------------*/
size_t flashGetGeometry(const void *object, struct FlashGeometry *geometry,
    size_t capacity)
{
  return flashBaseGetGeometry(object, geometry, capacity);
}
