/*
 * eeprom.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/eeprom.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/eeprom_defs.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <xcore/memory.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define EEPROM_CLOCK  1500000
#define PAGE_SIZE     128
/*----------------------------------------------------------------------------*/
static size_t calcChunkLength(uint32_t, size_t);
static inline bool isAddressValid(const struct Eeprom *, uint32_t);
static void programNextChunk(struct Eeprom *);
static bool setInstance(struct Eeprom *);
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *, const void *);
static void eepromDeinit(void *);
static void eepromSetCallback(void *, void (*)(void *), void *);
static enum Result eepromGetParam(void *, int, void *);
static enum Result eepromSetParam(void *, int, const void *);
static size_t eepromRead(void *, void *, size_t);
static size_t eepromWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Eeprom = &(const struct InterfaceClass){
    .size = sizeof(struct Eeprom),
    .init = eepromInit,
    .deinit = eepromDeinit,

    .setCallback = eepromSetCallback,
    .getParam = eepromGetParam,
    .setParam = eepromSetParam,
    .read = eepromRead,
    .write = eepromWrite
};
/*----------------------------------------------------------------------------*/
static struct Eeprom *instance = 0;
/*----------------------------------------------------------------------------*/
static size_t calcChunkLength(uint32_t address, size_t left)
{
  const uint32_t pageAddress = (address + PAGE_SIZE) & ~(PAGE_SIZE - 1);
  const size_t chunkLength = MIN(left, pageAddress - address);

  return chunkLength;
}
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Eeprom *interface,
    uint32_t address)
{
  return !(address & 0x03) && address < interface->base.size;
}
/*----------------------------------------------------------------------------*/
static void programNextChunk(struct Eeprom *interface)
{
  const size_t nextChunkLength = calcChunkLength(interface->offset,
      instance->left);

  memcpy((void *)interface->offset, interface->buffer, nextChunkLength);
  LPC_EEPROM->CMD = CMD_ERASE_PROGRAM;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Eeprom *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void EEPROM_ISR(void)
{
  /* Clear pending interrupt flag */
  LPC_EEPROM->INTSTATCLR = INT_PROG_DONE;

  const size_t currentChunkLength = calcChunkLength(instance->offset,
      instance->left);

  instance->buffer += currentChunkLength;
  instance->left -= currentChunkLength;
  instance->offset += currentChunkLength;

  if (instance->left)
    programNextChunk(instance);
}
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object, const void *configBase)
{
  const struct EepromConfig * const config = configBase;
  struct Eeprom * const interface = object;
  enum Result res;

  if ((res = EepromBase->init(interface, 0)) != E_OK)
    return res;

  if (!setInstance(interface))
    return E_BUSY;

  interface->position = 0;
  interface->blocking = true;

  /* Enable clock to register interface and peripheral */
  sysClockEnable(CLK_M4_EEPROM);
  /* Reset registers to default values */
  sysResetEnable(RST_EEPROM);

  const uint32_t frequency = clockFrequency(MainClock);

  LPC_EEPROM->CLKDIV = (frequency + (EEPROM_CLOCK - 1)) / EEPROM_CLOCK - 1;
  LPC_EEPROM->INTENSET = INT_PROG_DONE;

  if (config)
    irqSetPriority(EEPROM_IRQ, config->priority);
  irqEnable(EEPROM_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void eepromDeinit(void *object __attribute__((unused)))
{
  irqDisable(EEPROM_IRQ);
  sysClockDisable(CLK_M4_EEPROM);

  instance = 0;
}
/*----------------------------------------------------------------------------*/
static void eepromSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Eeprom * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result eepromGetParam(void *object, int parameter, void *data)
{
  struct Eeprom * const interface = object;

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
static enum Result eepromSetParam(void *object, int parameter, const void *data)
{
  struct Eeprom * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      if (isAddressValid(interface, position))
      {
        interface->position = position;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t eepromRead(void *object, void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  if (!isAddressValid(interface, interface->position + length))
    return 0;

  const uint32_t position = interface->base.address + interface->position;

  memcpy(buffer, (const void *)position, length);
  return length;
}
/*----------------------------------------------------------------------------*/
static size_t eepromWrite(void *object, const void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  /* Buffer size must be aligned along 4-byte boundary */
  assert(length % 4 == 0);

  if (!isAddressValid(interface, interface->position + length))
    return 0;

  interface->buffer = buffer;
  interface->left = length;
  interface->offset = interface->base.address + interface->position;

  programNextChunk(interface);

  if (interface->blocking)
  {
    while (interface->left)
      barrier();
  }

  return length;
}
