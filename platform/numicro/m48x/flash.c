/*
 * flash.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/platform/numicro/flash.h>
#include <halm/platform/numicro/flash_defs.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void accessLock(void);
static bool eraseBlock(uint32_t);
static bool erasePage(uint32_t);
static bool erasePageSPROM(void);
static bool isBlockPositionValid(const struct Flash *, uint32_t);
static bool isPagePositionValid(const struct Flash *, uint32_t);
static uint32_t positionToAddress(const struct Flash *, uint32_t);
static void readAccessUnlock(void);
static bool readFlash32(uint32_t, uint32_t *);
static bool readFlash64(uint32_t, uint64_t *);
static void writeAccessUnlock(const struct Flash *);
static bool writeFlash32(uint32_t, uint32_t);
static bool writeFlash64(uint32_t, uint64_t);
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
static void accessLock(void)
{
  /* Clear all bits except for BS and BL bits, clear ISPFF flag */
  NM_FMC->ISPCTL &= ISPCTL_BS | ISPCTL_ISPFF | ISPCTL_BL;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static bool eraseBlock(uint32_t address)
{
  NM_FMC->ISPCMD = CMD_FLASH_BLOCK_ERASE;
  NM_FMC->ISPADDR = address;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static bool erasePage(uint32_t address)
{
  NM_FMC->ISPCMD = CMD_FLASH_PAGE_ERASE;
  NM_FMC->ISPADDR = address;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static bool erasePageSPROM(void)
{
  NM_FMC->ISPCMD = CMD_FLASH_PAGE_ERASE;
  NM_FMC->ISPADDR = FLASH_SPROM_ADDRESS;
  NM_FMC->ISPDAT = ISPDAT_SPROM_ERASE_KEY;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static bool isBlockPositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= interface->size)
    return false;

  return (position & (FLASH_BLOCK_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
static bool isPagePositionValid(const struct Flash *interface,
    uint32_t position)
{
  if (position >= interface->size)
    return false;

  return (position & (FLASH_PAGE_SIZE - 1)) == 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t positionToAddress(const struct Flash *interface,
    uint32_t position)
{
  uint32_t offset = 0;

  switch (interface->bank)
  {
    case FLASH_BANK_0:
      offset = FLASH_BANK_0_ADDRESS;
      break;

    case FLASH_BANK_1:
      offset = FLASH_BANK_1_ADDRESS;
      break;

    case FLASH_CONFIG:
      offset = FLASH_CONFIG_ADDRESS;
      break;

    case FLASH_KPROM:
      offset = FLASH_KPROM_ADDRESS;
      break;

    case FLASH_LDROM:
      offset = FLASH_LDROM_ADDRESS;
      break;

    case FLASH_OTP:
      offset = FLASH_OTP_ADDRESS;
      break;

    case FLASH_SPROM:
      offset = FLASH_SPROM_ADDRESS;
      break;

    default:
      return 0;
  }

  return offset + position;
}
/*----------------------------------------------------------------------------*/
static void readAccessUnlock(void)
{
  sysUnlockReg();

  /* Preserve BS and BL bits, set ISPEN flag, clear ISPFF flag */
  NM_FMC->ISPCTL = (NM_FMC->ISPCTL & (ISPCTL_BS | ISPCTL_BL))
      | (ISPCTL_ISPEN | ISPCTL_ISPFF);
}
/*----------------------------------------------------------------------------*/
static bool readFlash32(uint32_t address, uint32_t *value)
{
  NM_FMC->ISPCMD = CMD_FLASH_READ_32;
  NM_FMC->ISPADDR = address;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
  {
    *value = NM_FMC->ISPDAT;
    return true;
  }
}
/*----------------------------------------------------------------------------*/
static bool readFlash64(uint32_t address, uint64_t *value)
{
  NM_FMC->ISPCMD = CMD_FLASH_READ_64;
  NM_FMC->ISPADDR = address;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
  {
    *value = NM_FMC->MPDAT0 | ((uint64_t)NM_FMC->MPDAT1 << 32);
    return true;
  }
}
/*----------------------------------------------------------------------------*/
static void writeAccessUnlock(const struct Flash *interface)
{
  /* Preserve BS and BL bits, set ISPEN flag, clear ISPFF flag */
  uint32_t value = (NM_FMC->ISPCTL & (ISPCTL_BS | ISPCTL_BL))
      | (ISPCTL_ISPEN | ISPCTL_ISPFF);

  if (interface->bank == FLASH_BANK_0 || interface->bank == FLASH_BANK_1)
    value |= ISPCTL_APUEN;
  else if (interface->bank == FLASH_CONFIG)
    value |= ISPCTL_CFGUEN;
  else if (interface->bank == FLASH_LDROM)
    value |= ISPCTL_LDUEN;

  sysUnlockReg();
  NM_FMC->ISPCTL = value;
}
/*----------------------------------------------------------------------------*/
static bool writeFlash32(uint32_t address, uint32_t value)
{
  NM_FMC->ISPCMD = CMD_FLASH_PROGRAM_32;
  NM_FMC->ISPADDR = address;
  NM_FMC->ISPDAT = value;
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static bool writeFlash64(uint32_t address, uint64_t value)
{
  NM_FMC->ISPCMD = CMD_FLASH_PROGRAM_64;
  NM_FMC->ISPADDR = address;
  NM_FMC->MPDAT0 = (uint32_t)value;
  NM_FMC->MPDAT1 = (uint32_t)(value >> 32);
  NM_FMC->ISPTRG = ISPTRG_ISPGO;

  while (NM_FMC->ISPSTS & ISPSTS_ISPBUSY);

  if (NM_FMC->ISPSTS & ISPSTS_ISPFF)
  {
    NM_FMC->ISPCTL |= ISPCTL_ISPFF;
    return false;
  }
  else
    return true;
}
/*----------------------------------------------------------------------------*/
static enum Result flashInit(void *object, const void *configBase)
{
  const struct FlashConfig * const config = configBase;
  assert(config != NULL);
  assert(config->bank < FLASH_BANK_END);

  const size_t capacity = sysGetSizeAPROM();
  struct Flash * const interface = object;
  enum Result res = E_OK;

  if (config->bank == FLASH_BANK_0 || config->bank == FLASH_BANK_1)
  {
    if (capacity == 512 * 1024)
    {
      /* Dual-bank configuration */
      interface->size = capacity / 2;
    }
    else if (capacity > 0)
    {
      /* Bank 1 is not available on parts with 128 or 256 Kbytes of Flash */
      if (config->bank == FLASH_BANK_0)
        interface->size = capacity;
      else
        res = E_VALUE;
    }
    else
    {
      /* Unknown part number */
      res = E_ERROR;
    }
  }
  else if (config->bank == FLASH_CONFIG)
  {
    interface->size = 16;
  }
  else if (config->bank == FLASH_KPROM)
  {
    interface->size = 8 * 1024;
  }
  else if (config->bank == FLASH_LDROM)
  {
    interface->size = 4 * 1024;
  }
  else if (config->bank == FLASH_OTP)
  {
    interface->size = 3 * 1024;
  }
  else if (config->bank == FLASH_SPROM)
  {
    if (capacity != 512 * 1024)
    {
      /* SPROM is not available on parts with 128 or 256 Kbytes of Flash */
      res = E_ERROR;
    }
    else
      interface->size = 4 * 1024;
  }
  else
  {
    res = E_VALUE;
  }

  if (res == E_OK)
  {
    interface->position = 0;
    interface->bank = config->bank;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result flashGetParam(void *object, int parameter, void *data)
{
  const struct Flash * const interface = object;

  /* Additional Flash parameters */
  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_BLOCK_SIZE:
      if (interface->bank == FLASH_BANK_0 || interface->bank == FLASH_BANK_1)
      {
        *(uint32_t *)data = FLASH_BLOCK_SIZE;
        return E_OK;
      }
      else
        return E_INVALID;

    case IF_FLASH_PAGE_SIZE:
      if (interface->bank == FLASH_CONFIG || interface->bank == FLASH_OTP)
        *(uint32_t *)data = interface->size;
      else
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
      *(uint32_t *)data = interface->size;
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
    case IF_FLASH_ERASE_BLOCK:
    {
      const uint32_t position = *(const uint32_t *)data;
      bool ok;

      if (interface->bank != FLASH_BANK_0 && interface->bank != FLASH_BANK_1)
        return E_INVALID;
      if (!isBlockPositionValid(interface, position))
        return E_ADDRESS;

      writeAccessUnlock(interface);
      ok = eraseBlock(positionToAddress(interface, position));
      accessLock();

      return ok ? E_OK : E_MEMORY;
    }

    case IF_FLASH_ERASE_PAGE:
    {
      const uint32_t position = *(const uint32_t *)data;
      bool ok;

      if (interface->bank == FLASH_OTP)
        return E_INVALID;
      if (!isPagePositionValid(interface, position))
        return E_ADDRESS;

      writeAccessUnlock(interface);
      if (interface->bank == FLASH_SPROM)
        ok = erasePageSPROM();
      else
        ok = erasePage(positionToAddress(interface, position));
      accessLock();

      return ok ? E_OK : E_MEMORY;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

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

  /* Address and length should be aligned along 4-byte boundary */
  if ((interface->position & 0x3) || (length & 0x3))
    return 0;

  /* Address should be inside the boundary */
  if (interface->position + length > interface->size)
    return 0;

  size_t left = length;
  uint32_t address = positionToAddress(interface, interface->position);
  uint8_t *output = buffer;

  readAccessUnlock();
  while (left)
  {
    if (!(address & 0x7))
    {
      uint64_t chunk;

      if (!readFlash64(address, &chunk))
        break;
      memcpy(output, &chunk, sizeof(chunk));

      left -= sizeof(chunk);
      output += sizeof(chunk);
      address += sizeof(chunk);
    }
    else
    {
      uint32_t chunk;

      if (!readFlash32(address, &chunk))
        break;
      memcpy(output, &chunk, sizeof(chunk));

      left -= sizeof(chunk);
      output += sizeof(chunk);
      address += sizeof(chunk);
    }
  }
  accessLock();

  interface->position += (uint32_t)(length - left);
  return length - left;
}
/*----------------------------------------------------------------------------*/
static size_t flashWrite(void *object, const void *buffer, size_t length)
{
  struct Flash * const interface = object;

  /* Address and length should be aligned along 4-byte boundary */
  if ((interface->position & 0x3) || (length & 0x3))
    return 0;

  /* Address should be inside the boundary */
  if (interface->position + length > interface->size)
    return 0;

  size_t left = length;
  uint32_t address = positionToAddress(interface, interface->position);
  const uint8_t *input = buffer;

  writeAccessUnlock(interface);
  while (left)
  {
    if (!(address & 0x7))
    {
      uint64_t chunk;

      memcpy(&chunk, input, sizeof(chunk));
      if (!writeFlash64(address, chunk))
        break;

      left -= sizeof(chunk);
      input += sizeof(chunk);
      address += sizeof(chunk);
    }
    else
    {
      uint32_t chunk;

      memcpy(&chunk, input, sizeof(chunk));
      if (!writeFlash32(address, chunk))
        break;

      left -= sizeof(chunk);
      input += sizeof(chunk);
      address += sizeof(chunk);
    }
  }
  accessLock();

  interface->position += (uint32_t)(length - left);
  return length - left;
}
/*----------------------------------------------------------------------------*/
size_t flashGetGeometry(const void *object, struct FlashGeometry *geometry,
    size_t capacity)
{
  if (!capacity)
    return 0;

  const struct Flash * const interface = object;

  if (interface->bank == FLASH_BANK_0 || interface->bank == FLASH_BANK_1)
  {
    geometry[0].count = interface->size / FLASH_BLOCK_SIZE;
    geometry[0].size = FLASH_BLOCK_SIZE;
    geometry[0].time = 20;

    return 1;
  }

  return 0;
}
