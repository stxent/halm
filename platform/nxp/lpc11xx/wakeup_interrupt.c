/*
 * wakeup_interrupt.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/generic/pointer_list.h>
#include <halm/platform/nxp/wakeup_interrupt.h>
/*----------------------------------------------------------------------------*/
struct StartLogicHandler
{
  struct Entity base;
  PointerList list;
};
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(struct PinData);
static enum Result startLogicHandlerAttach(struct PinData,
    struct WakeupInterrupt *);
static enum Result startLogicHandlerInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void startLogicHandlerDetach(struct WakeupInterrupt *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result wakeupInterruptInit(void *, const void *);
static void wakeupInterruptEnable(void *);
static void wakeupInterruptDisable(void *);
static void wakeupInterruptSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void wakeupInterruptDeinit(void *);
#else
#define wakeupInterruptDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const StartLogicHandler =
    &(const struct EntityClass){
    .size = sizeof(struct StartLogicHandler),
    .init = startLogicHandlerInit,
    .deinit = deletedDestructorTrap
};

const struct InterruptClass * const WakeupInterrupt =
    &(const struct InterruptClass){
    .size = sizeof(struct WakeupInterrupt),
    .init = wakeupInterruptInit,
    .deinit = wakeupInterruptDeinit,

    .enable = wakeupInterruptEnable,
    .disable = wakeupInterruptDisable,
    .setCallback = wakeupInterruptSetCallback
};
/*----------------------------------------------------------------------------*/
static struct StartLogicHandler *handler = 0;
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(struct PinData data)
{
  return WAKEUP_IRQ + data.port * 12 + data.offset;
}
/*----------------------------------------------------------------------------*/
void WAKEUP_ISR(void)
{
  PointerList * const list = &handler->list;
  PointerListNode *current = pointerListFront(list);
  const uint32_t state = LPC_SYSCON->STARTSRP0;

  /* Clear start-up logic states */
  LPC_SYSCON->STARTRSRP0CLR = state;

  while (current)
  {
    struct WakeupInterrupt * const interrupt = *pointerListData(current);
    const unsigned int index = interrupt->pin.port * 12 + interrupt->pin.offset;

    if (state & 1UL << (index & 0x1F))
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = pointerListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result startLogicHandlerAttach(struct PinData pin,
    struct WakeupInterrupt *interrupt)
{
  if (!handler)
    handler = init(StartLogicHandler, 0);

  assert(handler);

  PointerList * const list = &handler->list;
  PointerListNode *current = pointerListFront(list);

  /* Check for duplicates */
  while (current)
  {
    struct WakeupInterrupt * const entry = *pointerListData(current);

    if (entry->pin.port == pin.port && entry->pin.offset == pin.offset)
      return E_BUSY;

    current = pointerListNext(current);
  }

  /* Add to list */
  return pointerListPushFront(list, interrupt) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void startLogicHandlerDetach(struct WakeupInterrupt *interrupt)
{
  assert(pointerListFind(&handler->list, interrupt));
  pointerListErase(&handler->list, interrupt);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result startLogicHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct StartLogicHandler * const startLogicHandler = object;

  pointerListInit(&startLogicHandler->list);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result wakeupInterruptInit(void *object, const void *configBase)
{
  const struct WakeupInterruptConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  struct WakeupInterrupt * const interrupt = object;
  enum Result res;

  assert(config->event != PIN_TOGGLE);
  assert(pinValid(input));

  const unsigned int index = interrupt->pin.port * 12 + interrupt->pin.offset;
  assert(index <= 12);

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = startLogicHandlerAttach(input.data, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  const uint32_t mask = 1UL << index;

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
  /* Enable interrupt in NVIC, interrupt is masked by default */
  irqEnable(calcVector(interrupt->pin));

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void wakeupInterruptDeinit(void *object)
{
  const struct PinData data = ((struct WakeupInterrupt *)object)->pin;
  const unsigned int index = data.port * 12 + data.offset;

  irqDisable(calcVector(data));
  LPC_SYSCON->STARTERP0 &= ~(1UL << index);

  startLogicHandlerDetach(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void wakeupInterruptEnable(void *object)
{
  const struct PinData data = ((struct WakeupInterrupt *)object)->pin;
  const unsigned int index = data.port * 12 + data.offset;
  const uint32_t mask = 1UL << index;

  LPC_SYSCON->STARTRSRP0CLR = mask;
  LPC_SYSCON->STARTERP0 |= mask;
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptDisable(void *object)
{
  const struct PinData data = ((struct WakeupInterrupt *)object)->pin;
  const unsigned int index = data.port * 12 + data.offset;

  LPC_SYSCON->STARTERP0 &= ~(1UL << index);
}
/*----------------------------------------------------------------------------*/
static void wakeupInterruptSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct WakeupInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
