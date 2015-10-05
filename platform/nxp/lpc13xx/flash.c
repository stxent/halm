/*
 * flash.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <bits.h>
#include <platform/nxp/flash.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/flash_defs.h>
/*----------------------------------------------------------------------------*/
static inline int addressToSector(struct Flash *, uint32_t);
static enum iapResult iap(enum iapCommand, unsigned long *, unsigned short,
    const unsigned long *, unsigned short);
static enum result blankCheckSector(int);
static enum result compareRegions(uint32_t, const uint8_t *, uint32_t);
static enum result copyRamToFlash(uint32_t, const uint8_t *, uint32_t);
static enum result eraseSector(int);
static enum result prepareToWrite(int);
static uint32_t readPartId();
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *, const void *);
static void flashDeinit(void *);
static enum result flashCallback(void *, void (*)(void *), void *);
static enum result flashGet(void *, enum ifOption, void *);
static enum result flashSet(void *, enum ifOption, const void *);
static uint32_t flashRead(void *, uint8_t *, uint32_t);
static uint32_t flashWrite(void *, const uint8_t *, uint32_t);
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
static inline int addressToSector(struct Flash *interface, uint32_t address)
{
  if (address & (FLASH_PAGE_SIZE - 1))
    return -1;
  if (address > interface->size)
    return -1;

  return address / FLASH_SECTOR_SIZE;
}
/*----------------------------------------------------------------------------*/
static enum iapResult iap(enum iapCommand command, unsigned long *results,
    unsigned short resultsCount, const unsigned long *parameters,
    unsigned short parametersCount)
{
  if (resultsCount > 3 || parametersCount > 4)
    return RES_INVALID_COMMAND;

  unsigned long arguments[5] = {0};
  unsigned long resultBuffer[4];

  arguments[0] = (unsigned long)command;
  for (unsigned short index = 0; index < parametersCount; ++index)
    arguments[1 + index] = parameters[index];

  ((void (*)())IAP_BASE)(arguments, resultBuffer);

  for (unsigned short index = 0; index < resultsCount; ++index)
    results[index] = resultBuffer[1 + index];
  return (enum iapResult)resultBuffer[0];
}
/*----------------------------------------------------------------------------*/
static enum result blankCheckSector(int sector)
{
  const unsigned long parameters[] = {
      (unsigned long)sector,
      (unsigned long)sector
  };

  const enum iapResult res = iap(CMD_BLANK_CHECK_SECTORS, 0, 0, parameters,
      ARRAY_SIZE(parameters));

  return res == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result compareRegions(uint32_t address, const uint8_t *buffer,
    uint32_t length)
{
  const unsigned long parameters[] = {
      (unsigned long)address,
      (unsigned long)buffer,
      (unsigned long)length
  };

  const enum iapResult res = iap(CMD_COMPARE, 0, 0, parameters,
      ARRAY_SIZE(parameters));

  return res == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result copyRamToFlash(uint32_t address, const uint8_t *buffer,
    uint32_t length)
{
  const unsigned long parameters[] = {
      (unsigned long)address,
      (unsigned long)buffer,
      (unsigned long)length,
      (unsigned long)12000
  };

  const enum iapResult res = iap(CMD_COPY_RAM_TO_FLASH, 0, 0, parameters,
      ARRAY_SIZE(parameters));

  return res == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result eraseSector(int sector)
{
  const unsigned long parameters[] = {
      (unsigned long)sector,
      (unsigned long)sector,
      (unsigned long)12000
  };

  const enum iapResult res = iap(CMD_ERASE_SECTORS, 0, 0, parameters,
      ARRAY_SIZE(parameters));

  return res == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result prepareToWrite(int sector)
{
  const unsigned long parameters[] = {
      (unsigned long)sector,
      (unsigned long)sector
  };

  const enum iapResult res = iap(CMD_PREPARE_FOR_WRITE, 0, 0, parameters,
      ARRAY_SIZE(parameters));

  return res == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t readPartId()
{
  unsigned long id;

  iap(CMD_READ_PART_ID, &id, 1, 0, 0);
  return (uint32_t)id;
}
/*----------------------------------------------------------------------------*/
static enum result flashInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Flash * const interface = object;

  const uint32_t id = readPartId();

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

  interface->callback = 0;
  interface->position = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void flashDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result flashCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Flash * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result flashGet(void *object, enum ifOption option, void *data)
{
  struct Flash * const interface = object;

  /* Additional Flash options */
  switch ((enum flashOption)option)
  {
    case IF_FLASH_SECTOR_SIZE:
      /* Fixed sector size. */
      *(uint32_t *)data = 4 * 1024;
      return E_OK;

    default:
      break;
  }

  switch (option)
  {
    case IF_ADDRESS:
      *(uint32_t *)data = interface->position;
      return E_OK;

    case IF_ALIGNMENT:
      *(uint32_t *)data = FLASH_PAGE_SIZE;
      return E_OK;

    case IF_STATUS:
      return E_OK;

    case IF_SIZE:
      *((uint32_t *)data) = interface->size;
      return E_OK;

    default:
      return E_ERROR;
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
      const int sector = addressToSector(interface, *(const uint32_t *)data);

      if (sector == -1)
        return E_VALUE;
      if (blankCheckSector(sector) == E_OK) // TODO Distinguish error codes
        return E_OK;

      enum result res;

      if ((res = prepareToWrite(sector)) != E_OK)
        return res;
      if ((res = eraseSector(sector)) != E_OK)
        return res;
      if ((res = blankCheckSector(sector)) != E_OK)
        return res;

      return E_OK;
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_ADDRESS:
    {
      const uint32_t position = *(const uint32_t *)data;

      // TODO Remove redundant checks
      if (addressToSector(interface, position) == -1)
        return E_VALUE;

      interface->position = position;
      return E_OK;
    }

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t flashRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Flash * const interface = object;

  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;
  if (addressToSector(interface, interface->position) == -1)
    return 0;

  memcpy(buffer, (const void *)interface->position, length);

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t flashWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Flash * const interface = object;

  if (length & (FLASH_PAGE_SIZE - 1))
    return 0;

  const int sector = addressToSector(interface, interface->position);

  if (sector == -1)
    return 0;

  if (prepareToWrite(sector) != E_OK)
    return 0;
  if (copyRamToFlash(interface->position, buffer, length) != E_OK)
    return 0;
  if (compareRegions(interface->position, buffer, length) != E_OK)
    return 0;

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  return length;
}
