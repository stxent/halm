/*
 * eeprom.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/eeprom.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/eeprom_defs.h>
#include <halm/platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define EEPROM_CLOCK  1500000
#define PAGE_SIZE     128

extern unsigned long _seeprom;
extern unsigned long _eeeprom;
/*----------------------------------------------------------------------------*/
static size_t calcChunkLength(uintptr_t, size_t);
static inline bool isAddressValid(const struct Eeprom *, uintptr_t);
static void programNextChunk(struct Eeprom *);
static bool setDescriptor(const struct Eeprom *, struct Eeprom *);
/*----------------------------------------------------------------------------*/
static enum result eepromInit(void *, const void *);
static void eepromDeinit(void *);
static enum result eepromSetCallback(void *, void (*)(void *), void *);
static enum result eepromGetParam(void *, enum IfParameter, void *);
static enum result eepromSetParam(void *, enum IfParameter, const void *);
static size_t eepromRead(void *, void *, size_t);
static size_t eepromWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass eepromTable = {
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
const struct InterfaceClass * const Eeprom = &eepromTable;
static struct Eeprom *descriptor = 0;
/*----------------------------------------------------------------------------*/
static size_t calcChunkLength(uintptr_t address, size_t left)
{
  const uintptr_t pageAddress = (address + PAGE_SIZE) & ~(PAGE_SIZE - 1);
  const size_t chunkLength = pageAddress - address < left ?
      pageAddress - address : left;

  return chunkLength;
}
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Eeprom *interface,
    uintptr_t address)
{
  return !(address & 0x03) && address < interface->size;
}
/*----------------------------------------------------------------------------*/
static void programNextChunk(struct Eeprom *interface)
{
  const size_t nextChunkLength = calcChunkLength(interface->offset,
      descriptor->left);

  memcpy((void *)interface->offset, interface->buffer, nextChunkLength);
  LPC_EEPROM->CMD = CMD_ERASE_PROGRAM;
}
/*----------------------------------------------------------------------------*/
static bool setDescriptor(const struct Eeprom *state, struct Eeprom *interface)
{
  return compareExchangePointer((void **)&descriptor, state, interface);
}
/*----------------------------------------------------------------------------*/
void EEPROM_ISR(void)
{
  /* Clear pending interrupt flag */
  LPC_EEPROM->INTSTATCLR = INT_PROG_DONE;

  const size_t currentChunkLength = calcChunkLength(descriptor->offset,
      descriptor->left);

  descriptor->buffer += currentChunkLength;
  descriptor->left -= currentChunkLength;
  descriptor->offset += currentChunkLength;

  if (descriptor->left)
    programNextChunk(descriptor);
}
/*----------------------------------------------------------------------------*/
static enum result eepromInit(void *object, const void *configBase)
{
  const struct EepromConfig * const config = configBase;
  struct Eeprom * const interface = object;

  if (!setDescriptor(0, interface))
    return E_BUSY;

  interface->position = 0;
  interface->size = (uintptr_t)&_eeeprom - (uintptr_t)&_seeprom;
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
}
/*----------------------------------------------------------------------------*/
static enum result eepromSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Eeprom * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result eepromGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Eeprom * const interface = object;

  switch (parameter)
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
static enum result eepromSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Eeprom * const interface = object;

  switch (parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_POSITION:
    {
      const uintptr_t position = *(const uint32_t *)data;

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

  const uintptr_t position = (uintptr_t)&_seeprom + interface->position;

  memcpy(buffer, (const void *)position, length);
  return length;
}
/*----------------------------------------------------------------------------*/
static size_t eepromWrite(void *object, const void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  /* Buffer size must be aligned along 4-byte boundary */
  assert(!(length & 0x03));

  if (!isAddressValid(interface, interface->position + length))
    return 0;

  interface->buffer = buffer;
  interface->left = length;
  interface->offset = (uintptr_t)&_seeprom + interface->position;

  programNextChunk(interface);

  if (interface->blocking)
  {
    while (interface->left)
      barrier();
  }

  return length;
}
