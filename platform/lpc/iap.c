/*
 * iap.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/flash_defs.h>
#include <halm/platform/lpc/iap.h>
#include <halm/target.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
enum IapCommand
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
  CMD_EEPROM_READ             = 62
};

enum IapResult
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
static enum Result compareRegions(uint32_t, const void *, size_t);
static enum Result copyRamToFlash(uint32_t, const void *, size_t);
static enum Result iap(enum IapCommand, unsigned long *, size_t,
    const unsigned long *, size_t);
static enum Result prepareSectorToWrite(unsigned long, unsigned long);
/*----------------------------------------------------------------------------*/
static enum Result copyRamToFlash(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_COPY_RAM_TO_FLASH, NULL, 0,
      parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum Result compareRegions(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length
  };

  return iap(CMD_COMPARE, NULL, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum Result iap(enum IapCommand command, unsigned long *results,
    size_t resultsCount, const unsigned long *parameters,
    size_t parametersCount)
{
  unsigned long parameterBuffer[5] = {0};
  unsigned long resultBuffer[4] = {0};

  assert(parametersCount <= ARRAY_SIZE(parameterBuffer));
  assert(resultsCount <= ARRAY_SIZE(resultBuffer));

  parameterBuffer[0] = command;
  for (size_t index = 0; index < parametersCount; ++index)
    parameterBuffer[1 + index] = parameters[index];

  ((void (*)(const unsigned long *, unsigned long *))IAP_BASE)(parameterBuffer,
      resultBuffer);

  for (size_t index = 0; index < resultsCount; ++index)
    results[index] = resultBuffer[1 + index];

  return (enum IapResult)resultBuffer[0] == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result prepareSectorToWrite(unsigned long sector,
    unsigned long bank)
{
  const unsigned long parameters[] = {
      sector,
      sector,
      bank
  };

  return iap(CMD_PREPARE_FOR_WRITE, NULL, 0,
      parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum Result eepromReadBuffer(uint32_t address, void *buffer, size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_EEPROM_READ, NULL, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum Result eepromWriteBuffer(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_EEPROM_WRITE, NULL, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum Result flashActivateBootBank(unsigned int bank)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      (unsigned long)bank,
      frequency / 1000
  };

  return iap(CMD_SET_ACTIVE_BOOT_BANK, NULL, 0,
      parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum Result flashBlankCheckSector(uint32_t address, bool uniform)
{
  const unsigned long sector = addressToSector(address, uniform);
  const unsigned long parameters[] = {
      sector,
      sector,
      addressToBank(address)
  };

  return iap(CMD_BLANK_CHECK_SECTORS, NULL, 0,
      parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum Result flashErasePage(uint32_t address, bool uniform)
{
  const unsigned long sector = addressToSector(address, uniform);
  const unsigned long bank = addressToBank(address);
  enum Result res;

  const IrqState state = irqSave();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const unsigned long frequency = clockFrequency(MainClock);
    const unsigned long parameters[] = {
        addressToPage(address),
        addressToPage(address),
        frequency / 1000
    };

    res = iap(CMD_ERASE_PAGE, NULL, 0, parameters, ARRAY_SIZE(parameters));
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
enum Result flashEraseSector(uint32_t address, bool uniform)
{
  const unsigned long sector = addressToSector(address, uniform);
  const unsigned long bank = addressToBank(address);
  enum Result res;

  const IrqState state = irqSave();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const unsigned long frequency = clockFrequency(MainClock);
    const unsigned long parameters[] = {
        sector,
        sector,
        frequency / 1000,
        bank
    };

    res = iap(CMD_ERASE_SECTORS, NULL, 0,
        parameters, ARRAY_SIZE(parameters));
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
void flashInitWrite(void)
{
  iap(CMD_INIT, NULL, 0, NULL, 0);
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadId(void)
{
  unsigned long id[1];

  iap(CMD_READ_PART_ID, id, ARRAY_SIZE(id), NULL, 0);
  return id[0];
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadConfigId(void)
{
  unsigned long id[2];

  iap(CMD_READ_PART_ID, id, ARRAY_SIZE(id), NULL, 0);
  return id[1];
}
/*----------------------------------------------------------------------------*/
enum Result flashWriteBuffer(uint32_t address, bool uniform,
    const void *buffer, size_t length)
{
  const unsigned long sector = addressToSector(address, uniform);
  const unsigned long bank = addressToBank(address);
  enum Result res;

  const IrqState state = irqSave();

  res = prepareSectorToWrite(sector, bank);
  if (res == E_OK)
    res = copyRamToFlash(address, buffer, length);
  if (res == E_OK)
    res = compareRegions(address, buffer, length);

  irqRestore(state);
  return res;
}
