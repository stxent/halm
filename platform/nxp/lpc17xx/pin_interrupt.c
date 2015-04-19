/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <containers/list.h>
#include <platform/nxp/pin_interrupt.h>
#include <platform/nxp/lpc17xx/pin_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct PinInterruptHandler
{
  struct Entity parent;

  struct List list[2];
};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(uint8_t, union PinData);
static void enableInterrupt(uint8_t, union PinData, enum pinEvent);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(uint8_t, union PinData,
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
static void disableInterrupt(uint8_t channel, union PinData pin)
{
  const uint32_t mask = 1 << pin.offset;

  LPC_GPIO_INT->PORT[channel].ENF &= ~mask;
  LPC_GPIO_INT->PORT[channel].ENR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(uint8_t channel, union PinData pin,
    enum pinEvent event)
{
  const uint32_t mask = 1 << pin.offset;

  /* Clear pending interrupt flag */
  LPC_GPIO_INT->PORT[channel].CLR = mask;
  /* Configure edge sensitivity options */
  if (event != PIN_RISING)
    LPC_GPIO_INT->PORT[channel].ENF |= mask;
  if (event != PIN_FALLING)
    LPC_GPIO_INT->PORT[channel].ENR |= mask;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  const struct List * const list = &handler->list[channel];
  const uint32_t state = LPC_GPIO_INT->PORT[channel].STATR
      | LPC_GPIO_INT->PORT[channel].STATF;
  const struct ListNode *current = listFirst(list);
  struct PinInterrupt *interrupt;

  while (current)
  {
    listData(list, current, &interrupt);

    if (state & (1 << interrupt->pin.offset))
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = listNext(current);
  }

  LPC_GPIO_INT->PORT[channel].CLR = state;
}
/*----------------------------------------------------------------------------*/
void EINT3_ISR(void)
{
  if (LPC_GPIO_INT->STATUS & STATUS_P0INT)
    processInterrupt(0);

  if (LPC_GPIO_INT->STATUS & STATUS_P2INT)
    processInterrupt(1);
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(uint8_t channel, union PinData pin,
    const struct PinInterrupt *interrupt)
{
  if (!handler)
    handler = init(PinInterruptHandler, 0);

  assert(handler);

  struct List * const list = &handler->list[channel];
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
  const bool empty = listEmpty(&handler->list[0])
      && listEmpty(&handler->list[1]);
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
  struct List * const list = &handler->list[interrupt->channel];
  struct ListNode *current = listFirst(list);
  struct PinInterrupt *entry;

  while (current)
  {
    listData(list, current, &entry);

    if (entry == interrupt)
    {
      listErase(list, current);
      if (listEmpty(&handler->list[0]) && listEmpty(&handler->list[1]))
        irqDisable(EINT3_IRQ);
      break;
    }

    current = listNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PinInterruptHandler * const interruptHandler = object;
  enum result res;

  res = listInit(&interruptHandler->list[0], sizeof(struct PinInterrupt *));
  if (res != E_OK)
    return res;

  res = listInit(&interruptHandler->list[1], sizeof(struct PinInterrupt *));
  if (res != E_OK)
    return res;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *object, const void *configBase)
{
  const struct PinInterruptConfig * const config = configBase;
  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;
  enum result res;

  if (!pinValid(input))
    return E_VALUE;

  /* External interrupt functionality is available only on two ports */
  if (input.data.port != 0 && input.data.port != 2)
    return E_VALUE;
  /* Map ports 0 and 2 on channels 0 and 1 */
  interrupt->channel = input.data.port >> 1;

  /* Try to register pin interrupt in the interrupt handler */
  res = pinInterruptHandlerAttach(interrupt->channel, input.data, interrupt);
  if (res != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  enableInterrupt(interrupt->channel, interrupt->pin, interrupt->event);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  const struct PinInterrupt * const interrupt = object;

  disableInterrupt(interrupt->channel, interrupt->pin);
  pinInterruptHandlerDetach(interrupt);
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
  const struct PinInterrupt * const interrupt = object;

  if (state)
    enableInterrupt(interrupt->channel, interrupt->pin, interrupt->event);
  else
    disableInterrupt(interrupt->channel, interrupt->pin);
}
