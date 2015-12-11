/*
 * flash_common.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <bits.h>
#include <irq.h>
#include <libhalm/target.h>
#include <platform/nxp/flash.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/flash_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/clocking.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
enum iapCommand
{
  CMD_INIT                    = 49,
  CMD_PREPARE_FOR_WRITE       = 50,
  CMD_COPY_RAM_TO_FLASH       = 51,
  CMD_ERASE_SECTORS           = 52,
  CMD_BLANK_CHECK_SECTORS     = 53,
  CMD_READ_PART_ID            = 54,
  CMD_READ_BOOT_CODE_VERSION  = 55,
  CMD_COMPARE                 = 56,
  CMD_REINVOKE_ISP            = 57,
  CMD_READ_UID                = 58,
  CMD_ERASE_PAGE              = 59,
  CMD_SET_ACTIVE_BOOT_BANK    = 60,
  CMD_EEPROM_WRITE            = 61,
  CMD_EEPROM_READ             = 61
};

enum iapResult
{
  RES_CMD_SUCCESS             = 0,
  RES_INVALID_COMMAND         = 1,
  RES_SRC_ADDR_ERROR          = 2,
  RES_DST_ADDR_ERROR          = 3,
  RES_SRC_ADDR_NOT_MAPPED     = 4,
  RES_DST_ADDR_NOT_MAPPED     = 5,
  RES_COUNT_ERROR             = 6,
  RES_INVALID_SECTOR          = 7,
  RES_SECTOR_NOT_BLANK        = 8,
  RES_SECTOR_NOT_PREPARED     = 9,
  RES_COMPARE_ERROR           = 10,
  RES_BUSY                    = 11
};
/*----------------------------------------------------------------------------*/
static enum result compareRegions(uint32_t, const uint8_t *, uint32_t);
static enum result copyRamToFlash(uint32_t, const uint8_t *, uint32_t);
static enum result iap(enum iapCommand, uint32_t *, uint8_t, const uint32_t *,
    uint8_t);
static enum result prepareSectorToWrite(uint32_t, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result copyRamToFlash(uint32_t address, const uint8_t *buffer,
    uint32_t length)
{
  const uint32_t frequency = clockFrequency(MainClock);
  const uint32_t parameters[] = {
      address,
      (uint32_t)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_COPY_RAM_TO_FLASH, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum result compareRegions(uint32_t address, const uint8_t *buffer,
    uint32_t length)
{
  const uint32_t parameters[] = {
      address,
      (uint32_t)buffer,
      length
  };

  return iap(CMD_COMPARE, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum result iap(enum iapCommand command, uint32_t *results,
    uint8_t resultsCount, const uint32_t *parameters, uint8_t parametersCount)
{
  unsigned long parameterBuffer[5] = {0};
  unsigned long resultBuffer[4];

  parameterBuffer[0] = (uint32_t)command;
  for (uint8_t index = 0; index < parametersCount; ++index)
    parameterBuffer[1 + index] = (unsigned long)parameters[index];

  ((void (*)())IAP_BASE)(parameterBuffer, resultBuffer);

  for (uint8_t index = 0; index < resultsCount; ++index)
    results[index] = (uint32_t)resultBuffer[1 + index];

  return (enum iapResult)resultBuffer[0] == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result prepareSectorToWrite(uint32_t sector, uint8_t bank)
{
  const uint32_t parameters[] = {sector, sector, (uint32_t)bank};

  return iap(CMD_PREPARE_FOR_WRITE, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum result flashBlankCheckSector(uint32_t address)
{
  const uint32_t sector = addressToSector(address);
  const uint8_t bank = addressToBank(address);
  const uint32_t parameters[] = {sector, sector, (uint32_t)bank};

  return iap(CMD_BLANK_CHECK_SECTORS, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum result flashErasePage(uint32_t address)
{
  const uint32_t sector = addressToSector(address);
  const uint8_t bank = addressToBank(address);
  enum result res;

  interruptsDisable();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const uint32_t frequency = clockFrequency(MainClock);
    const uint32_t parameters[] = {
        address,
        address,
        frequency / 1000
    };

    res = iap(CMD_ERASE_PAGE, 0, 0, parameters, ARRAY_SIZE(parameters));
  }

  interruptsEnable();
  return res;
}
/*----------------------------------------------------------------------------*/
enum result flashEraseSector(uint32_t address)
{
  const uint32_t sector = addressToSector(address);
  const uint8_t bank = addressToBank(address);
  enum result res;

  interruptsDisable();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const uint32_t frequency = clockFrequency(MainClock);
    const uint32_t parameters[] = {
        sector,
        sector,
        frequency / 1000,
        (uint32_t)bank
    };

    res = iap(CMD_ERASE_SECTORS, 0, 0, parameters, ARRAY_SIZE(parameters));
  }

  interruptsEnable();
  return res;
}
/*----------------------------------------------------------------------------*/
void flashInitWrite()
{
  iap(CMD_INIT, 0, 0, 0, 0);
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadId()
{
  uint32_t id;

  iap(CMD_READ_PART_ID, &id, 1, 0, 0);
  return id;
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadConfigId()
{
  uint32_t id[2];

  iap(CMD_READ_PART_ID, id, ARRAY_SIZE(id), 0, 0);
  return id[1];
}
/*----------------------------------------------------------------------------*/
enum result flashWriteBuffer(uint32_t address, const uint8_t *buffer,
    uint32_t length)
{
  const uint32_t sector = addressToSector(address);
  const uint8_t bank = addressToBank(address);
  enum result res;

  interruptsDisable();

  res = prepareSectorToWrite(sector, bank);
  if (res == E_OK)
    res = copyRamToFlash(address, buffer, length);
  if (res == E_OK)
    res = compareRegions(address, buffer, length);

  interruptsEnable();
  return res;
}