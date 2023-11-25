/*
 * eeprom.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/eeprom.h>
#include <halm/platform/lpc/lpc43xx/eeprom_defs.h>
#include <assert.h>
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
static struct Eeprom *instance = NULL;
/*----------------------------------------------------------------------------*/
static size_t calcChunkLength(uint32_t address, size_t left)
{
  const uint32_t pageAddress = (address + PAGE_SIZE) & ~(PAGE_SIZE - 1);
  const size_t wordsLeft = (pageAddress - address) >> 2;
  const size_t chunkLength = MIN(left, wordsLeft);

  return chunkLength;
}
/*----------------------------------------------------------------------------*/
static inline bool isAddressValid(const struct Eeprom *interface,
    uint32_t address)
{
  return !(address & 0x03) && address <= interface->base.size;
}
/*----------------------------------------------------------------------------*/
static void programNextChunk(struct Eeprom *interface)
{
  const size_t length = calcChunkLength(interface->offset, instance->left);
  uint32_t * const address = (uint32_t *)interface->offset;
  const IrqState state = irqSave();

  for (size_t index = 0; index < length; ++index)
  {
    /* Write buffer using 4-byte words */
    address[index] = interface->buffer[index];
  }

  LPC_EEPROM->CMD = CMD_ERASE_PROGRAM;
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Eeprom *object)
{
  if (instance == NULL)
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
  if (!(LPC_EEPROM->INTSTAT & INT_PROG_DONE))
    return;

  /* Clear pending interrupt flag */
  LPC_EEPROM->INTSTATCLR = INT_PROG_DONE;

  const size_t currentChunkLength = calcChunkLength(instance->offset,
      instance->left);

  instance->left -= currentChunkLength;
  instance->buffer += currentChunkLength;
  instance->offset += currentChunkLength << 2;

  if (instance->left)
    programNextChunk(instance);
  else
    instance->position = instance->offset - instance->base.address;
}
/*----------------------------------------------------------------------------*/
static enum Result eepromInit(void *object, const void *configBase)
{
  const struct EepromConfig * const config = configBase;
  struct Eeprom * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  const enum Result res = EepromBase->init(interface, NULL);
  if (res != E_OK)
    return res;

  interface->position = 0;
  interface->blocking = true;

  /* Main clock frequency */
  const uint32_t frequency = clockFrequency(MainClock);
  /* Main clock period in nanoseconds */
  const uint32_t period = (1000000000UL - 1 + frequency) / frequency;

  LPC_EEPROM->PWRDWN = 0;
  LPC_EEPROM->CLKDIV = (frequency + (EEPROM_CLOCK - 1)) / EEPROM_CLOCK - 1;

  const uint32_t rphase1 = (RPHASE1_WAIT_TIME - 1 + period) / period;
  const uint32_t rphase2 = (RPHASE2_WAIT_TIME - 1 + period) / period;

  LPC_EEPROM->RWSTATE = RWSTATE_RPHASE1(rphase1 - 1)
      | RWSTATE_RPHASE2(rphase2 - 1);

  const uint32_t phase1 = (PHASE1_WAIT_TIME - 1 + period) / period;
  const uint32_t phase2 = (PHASE2_WAIT_TIME - 1 + period) / period;
  const uint32_t phase3 = (PHASE3_WAIT_TIME - 1 + period) / period;

  LPC_EEPROM->WSTATE = WSTATE_PHASE1(phase1 - 1) | WSTATE_PHASE2(phase2 - 1)
      | WSTATE_PHASE3(phase3 - 1);

  LPC_EEPROM->AUTOPROG = AUTOPROG_MODE(AUTOPROG_OFF);
  LPC_EEPROM->INTSTATCLR = INT_PROG_DONE;
  LPC_EEPROM->INTENSET = INT_PROG_DONE;

  if (config != NULL)
    irqSetPriority(EEPROM_IRQ, config->priority);
  irqEnable(EEPROM_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void eepromDeinit(void *object __attribute__((unused)))
{
  irqDisable(EEPROM_IRQ);
  instance = NULL;

  EepromBase->deinit(object);
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

  const uint32_t position = interface->position + interface->base.address;

  memcpy(buffer, (const void *)position, length);
  interface->position += length;

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t eepromWrite(void *object, const void *buffer, size_t length)
{
  struct Eeprom * const interface = object;

  /* Buffer size must be aligned on a 4-byte boundary */
  assert(length % sizeof(uint32_t) == 0);

  if (!isAddressValid(interface, interface->position + length))
    return 0;

  interface->left = length >> 2;
  interface->buffer = buffer;
  interface->offset = interface->position + interface->base.address;

  programNextChunk(interface);

  if (interface->blocking)
  {
    while (interface->left)
      barrier();
  }

  return length;
}
