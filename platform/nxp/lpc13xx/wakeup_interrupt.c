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
  struct Entity base;

  struct List list;
};
/*----------------------------------------------------------------------------*/
static inline irqNumber calcVector(struct PinData data);
/*----------------------------------------------------------------------------*/
static enum result startLogicHandlerAttach(struct PinData,
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
static inline irqNumber calcVector(struct PinData data)
{
  return WAKEUP_IRQ + data.port * 12 + data.offset;
}
/*----------------------------------------------------------------------------*/
void WAKEUP_ISR(void)
{
  const struct List * const list = &handler->list;
  const struct ListNode *current = listFirst(list);
  const uint32_t state[2] = {LPC_SYSCON->STARTSRP0, LPC_SYSCON->STARTSRP1};
  struct WakeupInterrupt *interrupt;

  /* Clear start-up logic states */
  LPC_SYSCON->STARTRSRP0CLR = state[0];
  LPC_SYSCON->STARTRSRP1CLR = state[1];

  while (current)
  {
    listData(list, current, &interrupt);

    const unsigned int index = interrupt->pin.port * 12 + interrupt->pin.offset;

    if (state[index >> 5] & 1 << (index & 0x1F))
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum result startLogicHandlerAttach(struct PinData pin,
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

    if (entry->pin.port == pin.port && entry->pin.offset == pin.offset)
      return E_BUSY;

    current = listNext(current);
  }

  /* Add to list */
  return listPush(list, &interrupt);
}
/*----------------------------------------------------------------------------*/
static void startLogicHandlerDetach(const struct WakeupInterrupt *interrupt)
{
  struct List * const list = &handler->list;
  struct ListNode * const node = listFind(list, &interrupt);

  if (node)
    listErase(list, node);
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

  assert(config->event != PIN_TOGGLE);
  assert(pinValid(input));

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = startLogicHandlerAttach(input.data, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  const unsigned int index = interrupt->pin.port * 12 + interrupt->pin.offset;
  const unsigned int group = index >> 5;
  const uint32_t mask = 1 << (index & 0x1F);

  /* Configure edge sensitivity options */
  switch (config->event)
  {
    case PIN_RISING:
      LPC_SYSCON->START[group].APRP |= mask;
      break;

    case PIN_FALLING:
      LPC_SYSCON->START[group].APRP &= ~mask;
      break;

    default:
      break;
  }
  /* Clear status flag */
  LPC_SYSCON->START[group].RSRPCLR = mask;
  /* Enable interrupt in NVIC, interrupt is masked by default */
  irqEnable(calcVector(interrupt->pin));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptDeinit(void *object)
{
  const struct PinData data = ((struct WakeupInterrupt *)object)->pin;
  const unsigned int index = data.port * 12 + data.offset;
  const unsigned int group = index >> 5;

  irqDisable(calcVector(data));
  LPC_SYSCON->START[group].ERP &= ~(1 << (index & 0x1F));

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
  const struct PinData data = ((struct WakeupInterrupt *)object)->pin;
  const unsigned int index = data.port * 12 + data.offset;
  const unsigned int group = index >> 5;
  const uint32_t mask = 1 << (index & 0x1F);

  if (state)
  {
    LPC_SYSCON->START[group].RSRPCLR = mask;
    LPC_SYSCON->START[group].ERP |= mask;
  }
  else
    LPC_SYSCON->START[group].ERP &= ~mask;
}
