/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <list.h>
#include <platform/nxp/pin_interrupt.h>
#include <platform/nxp/lpc17xx/pin_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct PinInterruptHandler
{
  struct Entity parent;

  struct List list0;
  struct List list2;
};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(union PinData);
static void enableInterrupt(union PinData, enum pinEvent);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(union PinData,
    const struct PinInterrupt *);
static void pinInterruptHandlerDetach(const struct PinInterrupt *);
static enum result pinInterruptHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *, const void *);
static void pinInterruptDeinit(void *);
static void pinInterruptCallback(void *, void (*)(void *), void *);
static void pinInterruptSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct PinInterruptHandler),
    .init = pinInterruptHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct InterruptClass pinInterruptTable = {
    .size = sizeof(struct PinInterrupt),
    .init = pinInterruptInit,
    .deinit = pinInterruptDeinit,

    .callback = pinInterruptCallback,
    .setEnabled = pinInterruptSetEnabled
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PinInterruptHandler = &handlerTable;
const struct InterruptClass * const PinInterrupt = &pinInterruptTable;
static struct PinInterruptHandler *handler = 0;
/*----------------------------------------------------------------------------*/
static void disableInterrupt(union PinData pin)
{
  const uint32_t mask = 1 << pin.offset;

  switch (pin.port)
  {
    case 0:
      LPC_GPIO_INT->ENF0 &= ~mask;
      LPC_GPIO_INT->ENR0 &= ~mask;
      break;

    case 2:
      LPC_GPIO_INT->ENF2 &= ~mask;
      LPC_GPIO_INT->ENR2 &= ~mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(union PinData pin, enum pinEvent event)
{
  const uint32_t mask = 1 << pin.offset;

  switch (pin.port)
  {
    case 0:
      /* Clear pending interrupt flag */
      LPC_GPIO_INT->CLR0 = mask;
      /* Configure edge sensitivity options */
      if (event != PIN_RISING)
        LPC_GPIO_INT->ENF0 |= mask;
      if (event != PIN_FALLING)
        LPC_GPIO_INT->ENR0 |= mask;
      break;

    case 2:
      LPC_GPIO_INT->CLR2 = mask;
      if (event != PIN_RISING)
        LPC_GPIO_INT->ENF2 |= mask;
      if (event != PIN_FALLING)
        LPC_GPIO_INT->ENR2 |= mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  const struct List *list;
  volatile uint32_t *cleaner;
  uint32_t state;

  switch (channel)
  {
    case 0:
      cleaner = &LPC_GPIO_INT->CLR0;
      state = LPC_GPIO_INT->STATR0 | LPC_GPIO_INT->STATF0;
      list = &handler->list0;
      break;

    case 2:
      cleaner = &LPC_GPIO_INT->CLR2;
      state = LPC_GPIO_INT->STATR2 | LPC_GPIO_INT->STATF2;
      list = &handler->list2;
      break;
  }

  const struct ListNode *current = listFirst(list);
  struct PinInterrupt *interrupt;

  while (current)
  {
    listData(list, current, &interrupt);

    if (state & (1 << interrupt->pin.offset))
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
      break;
    }

    current = listNext(current);
  }

  *cleaner = state;
}
/*----------------------------------------------------------------------------*/
void EINT3_ISR(void)
{
  if (LPC_GPIO_INT->STATUS & STATUS_P0INT)
    processInterrupt(0);

  if (LPC_GPIO_INT->STATUS & STATUS_P2INT)
    processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(union PinData pin,
    const struct PinInterrupt *interrupt)
{
  if (!handler)
    handler = init(PinInterruptHandler, 0);

  assert(handler);

  struct List *list;

  switch (pin.port)
  {
    case 0:
      list = &handler->list0;
      break;

    case 2:
      list = &handler->list2;
      break;

    default:
      /* External interrupt functionality is available only on two ports */
      return E_VALUE;
  }

  const struct ListNode *current = listFirst(list);
  struct PinInterrupt *entry;

  /* Check for duplicates */
  while (current)
  {
    listData(list, current, &entry);

    if (entry->pin.key == pin.key)
      return E_BUSY;

    current = listNext(current);
  }

  /* Add to list */
  const bool empty = listEmpty(&handler->list0) && listEmpty(&handler->list2);
  const enum result res = listPush(list, &interrupt);

  if (res == E_OK && empty)
  {
    /* Initial interrupt configuration */
    sysClockControl(CLK_GPIOINT, DEFAULT_DIV);
    irqEnable(EINT3_IRQ);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptHandlerDetach(const struct PinInterrupt *interrupt)
{
  struct List * const list = !interrupt->pin.port ? &handler->list0
      : &handler->list2;
  struct ListNode *current = listFirst(list);
  struct PinInterrupt *entry;

  while (current)
  {
    listData(list, current, &entry);

    if (entry == interrupt)
    {
      listErase(list, current);
      if (listEmpty(&handler->list0) && listEmpty(&handler->list2))
        irqDisable(EINT3_IRQ);
      break;
    }

    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct PinInterruptHandler * const handler = object;
  enum result res;

  if ((res = listInit(&handler->list0, sizeof(struct PinInterrupt *))) != E_OK)
    return res;

  if ((res = listInit(&handler->list2, sizeof(struct PinInterrupt *))) != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *object, const void *configPtr)
{
  const struct PinInterruptConfig * const config = configPtr;
  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;
  enum result res;

  if (!pinGetKey(input))
    return E_VALUE;

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = pinInterruptHandlerAttach(input.data, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  enableInterrupt(interrupt->pin, interrupt->event);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  const union PinData pin = ((struct PinInterrupt *)object)->pin;

  disableInterrupt(pin);
  pinInterruptHandlerDetach(object);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptSetEnabled(void *object, bool state)
{
  struct PinInterrupt * const interrupt = object;

  if (state)
    enableInterrupt(interrupt->pin, interrupt->event);
  else
    disableInterrupt(interrupt->pin);
}
