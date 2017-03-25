/*
 * iap.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <halm/irq.h>
#include <halm/platform/nxp/iap.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/flash_defs.h>
#include HEADER_PATH
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/clocking.h>
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
  CMD_EEPROM_READ             = 62
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
static enum result compareRegions(uint32_t, const void *, size_t);
static enum result copyRamToFlash(uint32_t, const void *, size_t);
static enum result iap(enum iapCommand, unsigned long *, unsigned int,
    const unsigned long *, unsigned int);
static enum result prepareSectorToWrite(unsigned int, unsigned int);
/*----------------------------------------------------------------------------*/
static enum result copyRamToFlash(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_COPY_RAM_TO_FLASH, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum result compareRegions(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length
  };

  return iap(CMD_COMPARE, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
static enum result iap(enum iapCommand command, unsigned long *results,
    unsigned int resultsCount, const unsigned long *parameters,
    unsigned int parametersCount)
{
  unsigned long parameterBuffer[5] = {0};
  unsigned long resultBuffer[4] = {0};

  assert(parametersCount <= ARRAY_SIZE(parameterBuffer));
  assert(resultsCount <= ARRAY_SIZE(resultBuffer));

  parameterBuffer[0] = command;
  for (unsigned int index = 0; index < parametersCount; ++index)
    parameterBuffer[1 + index] = parameters[index];

  ((void (*)())IAP_BASE)(parameterBuffer, resultBuffer);

  for (unsigned int index = 0; index < resultsCount; ++index)
    results[index] = resultBuffer[1 + index];

  return (enum iapResult)resultBuffer[0] == RES_CMD_SUCCESS ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result prepareSectorToWrite(unsigned int sector, unsigned int bank)
{
  const unsigned long parameters[] = {sector, sector, bank};

  return iap(CMD_PREPARE_FOR_WRITE, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum result eepromReadBuffer(uint32_t address, void *buffer, size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_EEPROM_READ, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum result eepromWriteBuffer(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned long frequency = clockFrequency(MainClock);
  const unsigned long parameters[] = {
      address,
      (unsigned long)buffer,
      length,
      frequency / 1000
  };

  return iap(CMD_EEPROM_WRITE, 0, 0, parameters, ARRAY_SIZE(parameters));
}

/*----------------------------------------------------------------------------*/
enum result flashBlankCheckSector(uint32_t address)
{
  const unsigned int sector = addressToSector(address);
  const unsigned int bank = addressToBank(address);
  const unsigned long parameters[] = {
      sector,
      sector,
      bank
  };

  return iap(CMD_BLANK_CHECK_SECTORS, 0, 0, parameters, ARRAY_SIZE(parameters));
}
/*----------------------------------------------------------------------------*/
enum result flashErasePage(uint32_t address)
{
  const unsigned int sector = addressToSector(address);
  const unsigned int bank = addressToBank(address);
  enum result res;

  const irqState state = irqSave();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const unsigned long frequency = clockFrequency(MainClock);
    const unsigned long parameters[] = {
        addressToPage(address),
        addressToPage(address),
        frequency / 1000
    };

    res = iap(CMD_ERASE_PAGE, 0, 0, parameters, ARRAY_SIZE(parameters));
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
enum result flashEraseSector(uint32_t address)
{
  const unsigned int sector = addressToSector(address);
  const unsigned int bank = addressToBank(address);
  enum result res;

  const irqState state = irqSave();

  if ((res = prepareSectorToWrite(sector, bank)) == E_OK)
  {
    const unsigned long frequency = clockFrequency(MainClock);
    const unsigned long parameters[] = {
        sector,
        sector,
        frequency / 1000,
        bank
    };

    res = iap(CMD_ERASE_SECTORS, 0, 0, parameters, ARRAY_SIZE(parameters));
  }

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
void flashInitWrite(void)
{
  iap(CMD_INIT, 0, 0, 0, 0);
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadId(void)
{
  unsigned long id[1];

  iap(CMD_READ_PART_ID, id, ARRAY_SIZE(id), 0, 0);

  return id[0];
}
/*----------------------------------------------------------------------------*/
uint32_t flashReadConfigId(void)
{
  unsigned long id[2];

  iap(CMD_READ_PART_ID, id, ARRAY_SIZE(id), 0, 0);

  return id[1];
}
/*----------------------------------------------------------------------------*/
enum result flashWriteBuffer(uint32_t address, const void *buffer,
    size_t length)
{
  const unsigned int sector = addressToSector(address);
  const unsigned int bank = addressToBank(address);
  enum result res;

  const irqState state = irqSave();

  res = prepareSectorToWrite(sector, bank);
  if (res == E_OK)
    res = copyRamToFlash(address, buffer, length);
  if (res == E_OK)
    res = compareRegions(address, buffer, length);

  irqRestore(state);
  return res;
}
