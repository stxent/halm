/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <list.h>
#include <platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
struct PinInterruptHandler
{
  struct Entity parent;

  struct List list;
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static inline irq_t calcVector(uint8_t);
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
static struct PinInterruptHandler *handlers[4] = {0};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t port)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0 +
      ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * port);
}
/*----------------------------------------------------------------------------*/
static inline irq_t calcVector(uint8_t port)
{
  return PIOINT0_IRQ - port;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  const struct List * const list = &handlers[channel]->list;
  const struct ListNode *current = listFirst(list);
  LPC_GPIO_Type * const reg = calcPort(channel);
  const uint32_t state = reg->MIS;
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

  /* Note that synchronizer logic causes a delay of 2 clocks */
  reg->IC = state; //FIXME Add check
}
/*----------------------------------------------------------------------------*/
void PIOINT0_ISR(void)
{
  processInterrupt(0);
}
/*----------------------------------------------------------------------------*/
void PIOINT1_ISR(void)
{
  processInterrupt(1);
}
/*----------------------------------------------------------------------------*/
void PIOINT2_ISR(void)
{
  processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
void PIOINT3_ISR(void)
{
  processInterrupt(3);
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(union PinData pin,
    const struct PinInterrupt *interrupt)
{
  if (!handlers[pin.port])
    handlers[pin.port] = init(PinInterruptHandler, 0);

  assert(handlers[pin.port]);

  struct List * const list = &handlers[pin.port]->list;
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
  const bool empty = listEmpty(list);
  const enum result res = listPush(list, &interrupt);

  if (res == E_OK && empty)
    irqEnable(calcVector(pin.port));

  return res;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptHandlerDetach(const struct PinInterrupt *interrupt)
{
  struct List * const list = &handlers[interrupt->pin.port]->list;
  struct ListNode *current = listFirst(list);
  struct PinInterrupt *entry;

  while (current)
  {
    listData(list, current, &entry);

    if (entry == interrupt)
    {
      listErase(list, current);
      if (listEmpty(list))
        irqDisable(calcVector(interrupt->pin.port));
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

  return listInit(&handler->list, sizeof(struct PinInterrupt *));
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *object, const void *configPtr)
{
  const struct PinInterruptConfig * const config = configPtr;
  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;
  enum result res;

  if (!pinValid(input))
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

  LPC_GPIO_Type * const reg = calcPort(interrupt->pin.port);
  const uint32_t mask = 1 << interrupt->pin.offset;

  /* Configure interrupt as edge sensitive*/
  reg->IS &= ~mask;
  /* Configure edge sensitivity options */
  switch (config->event)
  {
    case PIN_RISING:
      reg->IEV |= mask;
      break;

    case PIN_FALLING:
      reg->IEV &= ~mask;
      break;

    case PIN_TOGGLE:
      reg->IBE |= mask;
      break;
  }
  /* Clear pending interrupt flag */
  reg->IC = mask;
  /* Disable interrupt masking */
  reg->IE |= mask;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  const union PinData data = ((struct PinInterrupt *)object)->pin;
  const uint32_t mask = 1 << data.offset;
  LPC_GPIO_Type * const reg = calcPort(data.port);

  reg->IE &= ~mask;
  reg->IBE &= ~mask;
  reg->IEV &= ~mask;
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
  const union PinData data = ((struct PinInterrupt *)object)->pin;
  const uint32_t mask = 1 << data.offset;
  LPC_GPIO_Type * const reg = calcPort(data.port);

  if (state)
  {
    reg->IC = mask;
    reg->IE |= mask;
  }
  else
    reg->IE &= ~mask;
}
