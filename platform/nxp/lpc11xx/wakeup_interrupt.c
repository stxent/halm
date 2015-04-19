/*
 * wakeup_interrupt.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <containers/list.h>
#include <platform/nxp/wakeup_interrupt.h>
/*----------------------------------------------------------------------------*/
struct StartLogicHandler
{
  struct Entity parent;

  struct List list;
};
/*----------------------------------------------------------------------------*/
static inline irq_t calcVector(union PinData data);
/*----------------------------------------------------------------------------*/
static enum result startLogicHandlerAttach(union PinData,
    const struct WakeupInterrupt *);
static void startLogicHandlerDetach(const struct WakeupInterrupt *);
static enum result startLogicHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static enum result wakeupInterruptInit(void *, const void *);
static void wakeupInterruptDeinit(void *);
static void wakeupInterruptCallback(void *, void (*)(void *), void *);
static void wakeupInterruptSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct StartLogicHandler),
    .init = startLogicHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const struct InterruptClass wakeupInterruptTable = {
    .size = sizeof(struct WakeupInterrupt),
    .init = wakeupInterruptInit,
    .deinit = wakeupInterruptDeinit,

    .callback = wakeupInterruptCallback,
    .setEnabled = wakeupInterruptSetEnabled
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const StartLogicHandler = &handlerTable;
const struct InterruptClass * const WakeupInterrupt = &wakeupInterruptTable;
static struct StartLogicHandler *handler = 0;
/*----------------------------------------------------------------------------*/
static inline irq_t calcVector(union PinData data)
{
  return WAKEUP_IRQ + data.port * 12 + data.offset;
}
/*----------------------------------------------------------------------------*/
void WAKEUP_ISR(void)
{
  const struct List * const list = &handler->list;
  const struct ListNode *current = listFirst(list);
  const uint32_t state = LPC_SYSCON->STARTSRP0;
  struct WakeupInterrupt *interrupt;

  /* Clear start-up logic states */
  LPC_SYSCON->STARTRSRP0CLR = state;

  while (current)
  {
    listData(list, current, &interrupt);

    const uint8_t index = interrupt->pin.port * 12 + interrupt->pin.offset;

    if (state & 1 << (index & 0x1F))
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum result startLogicHandlerAttach(union PinData pin,
    const struct WakeupInterrupt *interrupt)
{
  if (!handler)
    handler = init(StartLogicHandler, 0);

  assert(handler);

  struct List * const list = &handler->list;
  const struct ListNode *current = listFirst(list);
  struct WakeupInterrupt *entry;

  /* Check for duplicates */
  while (current)
  {
    listData(list, current, &entry);

    if (entry->pin.key == pin.key)
      return E_BUSY;

    current = listNext(current);
  }

  /* Add to list */
  const enum result res = listPush(list, &interrupt);

  if (res == E_OK)
    irqEnable(calcVector(pin));

  return res;
}
/*----------------------------------------------------------------------------*/
static void startLogicHandlerDetach(const struct WakeupInterrupt *interrupt)
{
  struct List * const list = &handler->list;
  struct ListNode *current = listFirst(list);
  struct WakeupInterrupt *entry;

  while (current)
  {
    listData(list, current, &entry);

    if (entry == interrupt)
    {
      listErase(list, current);
      if (listEmpty(list))
        irqDisable(calcVector(interrupt->pin));
      break;
    }

    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum result startLogicHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct StartLogicHandler * const startLogicHandler = object;

  return listInit(&startLogicHandler->list, sizeof(struct WakeupInterrupt *));
}
/*----------------------------------------------------------------------------*/
static enum result wakeupInterruptInit(void *object, const void *configBase)
{
  const struct WakeupInterruptConfig * const config = configBase;
  const struct Pin input = pinInit(config->pin);
  struct WakeupInterrupt * const interrupt = object;
  enum result res;

  if (config->event == PIN_TOGGLE)
    return E_INVALID;

  const uint8_t index = interrupt->pin.port * 12 + interrupt->pin.offset;

  if (!pinValid(input) || index > 12)
    return E_VALUE;

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = startLogicHandlerAttach(input.data, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  const uint32_t mask = 1 << index;

  /* Configure edge sensitivity options */
  switch (config->event)
  {
    case PIN_RISING:
      LPC_SYSCON->STARTAPRP0 |= mask;
      break;

    case PIN_FALLING:
      LPC_SYSCON->STARTAPRP0 &= ~mask;
      break;

    default:
      break;
  }
  /* Clear status flag */
  LPC_SYSCON->STARTRSRP0CLR = mask;
  /* Enable start signal for start logic input */
  LPC_SYSCON->STARTERP0 |= mask;
  /* Enable interrupt */
  irqEnable(calcVector(interrupt->pin));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptDeinit(void *object)
{
  const union PinData data = ((struct WakeupInterrupt *)object)->pin;
  const uint8_t index = data.port * 12 + data.offset;

  irqDisable(calcVector(data));
  LPC_SYSCON->STARTERP0 &= ~(1 << index);

  startLogicHandlerDetach(object);
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct WakeupInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptSetEnabled(void *object, bool state)
{
  const union PinData data = ((struct WakeupInterrupt *)object)->pin;
  const uint8_t index = data.port * 12 + data.offset;
  const uint32_t mask = 1 << index;

  if (state)
  {
    LPC_SYSCON->STARTRSRP0CLR = mask;
    LPC_SYSCON->STARTERP0 |= mask;
  }
  else
    LPC_SYSCON->STARTERP0 &= ~mask;
}
