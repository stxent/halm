/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/lpc/flash.h>
#include <halm/platform/lpc/flash_defs.h>
#include <halm/platform/lpc/iap.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static enum FlashBank getBankByAddress(uintptr_t);
static bool isPagePositionValid(const struct Flash *, uint32_t);
static bool isSectorPositionValid(const struct Flash *, uint32_t);
static uint32_t positionToAddress(const struct Flash *, uint32_t);
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
static enum FlashBank getBankByAddress(uintptr_t address)
{
  if (address >= FLASH_BANK_A_ADDRESS
      && address < FLASH_BANK_A_ADDRESS + FLASH_BANK_BORDER)
  {
    return FLASH_BANK_A;
  }
  else if (address >= FLASH_BANK_B_ADDRESS
      && address < FLASH_BANK_B_ADDRESS + FLASH_BANK_BORDER)
  {
    return FLASH_BANK_B;
  }
  else
    return FLASH_BANK_END;
}
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= interface->base.size)
    return false;
  if (position & (FLASH_PAGE_SIZE - 1))
    return false;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool isSectorPositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= interface->base.size)
    return false;

  const uint32_t mask = position < FLASH_SECTORS_BORDER ?
      (FLASH_SECTOR_SIZE_0 - 1) : (FLASH_SECTOR_SIZE_1 - 1);

  return (position & mask) == 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t positionToAddress(const struct Flash *interface,
    uint32_t position)
{
  return position + (interface->base.bank == FLASH_BANK_A ?
      FLASH_BANK_A_ADDRESS : FLASH_BANK_B_ADDRESS);
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashConfig * const config = configBase;
  assert(config == NULL || config->bank < FLASH_BANK_END);

  struct Flash * const interface = object;
  enum FlashBank bank;

  if (config == NULL || config->bank > FLASH_BANK_B)
  {
    /* Detect current bank */
    bank = getBankByAddress((uintptr_t)Flash);
    if (bank == FLASH_BANK_END)
      return E_ERROR;

    if (config != NULL && config->bank == FLASH_BANK_SPARE)
      bank = (bank == FLASH_BANK_A) ? FLASH_BANK_B : FLASH_BANK_A;
  }
  else
    bank = config->bank;

  const struct FlashBaseConfig baseConfig = {
      .bank = (uint8_t)bank
  };
  const enum Result res = FlashBase->init(interface, &baseConfig);

  if (res == E_OK)
    interface->position = 0;

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result flashGetParam(void *object, int parameter, void *data)
{
  const struct Flash * const interface = object;

  /* Additional Flash parameters */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_SECTOR_SIZE:
      /* Sector size is ambiguous, use geometry to calculate actual size */
      *(uint32_t *)data = 0;
      return E_OK;

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

  /* Additional Flash options */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (isSectorPositionValid(interface, position))
      {
        const uint32_t address = positionToAddress(interface, position);

        if (flashBlankCheckSector(address, interface->base.uniform) == E_OK)
          return E_OK;

        flashInitWrite();
        return flashEraseSector(address, interface->base.uniform);
      }
      else
        return E_ADDRESS;
    }

    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (isPagePositionValid(interface, position))
      {
        const uint32_t address = positionToAddress(interface, position);

        flashInitWrite();
        return flashErasePage(address, interface->base.uniform);
      }
      else
        return E_ADDRESS;
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
    const uint32_t address = positionToAddress(interface, interface->position);

    memcpy(buffer, (const void *)address, length);
    interface->position += (uint32_t)length;
    return length;
  }

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
  uint32_t address = positionToAddress(interface, interface->position);
  const uint8_t *input = buffer;

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
void *flashGetAddress(const void *object)
{
  const struct Flash * const interface = object;

  return interface->base.bank == FLASH_BANK_A ?
      (void *)FLASH_BANK_A_ADDRESS : (void *)FLASH_BANK_B_ADDRESS;
}
/*----------------------------------------------------------------------------*/
size_t flashGetGeometry(const void *object, struct FlashGeometry *geometry,
    size_t capacity)
{
  if (capacity < 2)
    return 0;

  const struct Flash * const interface = object;

  geometry[0].count = FLASH_SECTORS_BORDER / FLASH_SECTOR_SIZE_0;
  geometry[0].size = FLASH_SECTOR_SIZE_0;
  geometry[0].time = 100;

  geometry[1].count =
      (interface->base.size - FLASH_SECTORS_BORDER) / FLASH_SECTOR_SIZE_1;
  geometry[1].size = FLASH_SECTOR_SIZE_1;
  geometry[1].time = 100;

  return 2;
}
