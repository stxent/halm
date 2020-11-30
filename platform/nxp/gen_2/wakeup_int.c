/*
 * wakeup_int.c
 * Copyright (C) 2015, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/generic/pointer_list.h>
#include <halm/platform/nxp/gen_2/wakeup_int.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct StartLogicHandler
{
  struct Entity base;
  PointerList list;
};
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(uint8_t);
static enum Result startLogicHandlerAttach(uint8_t, struct WakeupInt *);
static enum Result startLogicHandlerInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void startLogicHandlerDetach(struct WakeupInt *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result wakeupIntInit(void *, const void *);
static void wakeupIntEnable(void *);
static void wakeupIntDisable(void *);
static void wakeupIntSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void wakeupIntDeinit(void *);
#else
#define wakeupIntDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const StartLogicHandler =
    &(const struct EntityClass){
    .size = sizeof(struct StartLogicHandler),
    .init = startLogicHandlerInit,
    .deinit = deletedDestructorTrap
};

const struct InterruptClass * const WakeupInt =
    &(const struct InterruptClass){
    .size = sizeof(struct WakeupInt),
    .init = wakeupIntInit,
    .deinit = wakeupIntDeinit,

    .enable = wakeupIntEnable,
    .disable = wakeupIntDisable,
    .setCallback = wakeupIntSetCallback
};
/*----------------------------------------------------------------------------*/
static struct StartLogicHandler *handler = 0;
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(uint8_t channel)
{
  return WAKEUP_IRQ + channel;
}
/*----------------------------------------------------------------------------*/
void WAKEUP_ISR(void)
{
  PointerList * const list = &handler->list;
  PointerListNode *current = pointerListFront(list);
  uint32_t state[ARRAY_SIZE(LPC_SYSCON->START)];

  /* Clear start-up logic states */
  for (size_t index = 0; index < ARRAY_SIZE(state); ++index)
  {
    state[index] = LPC_SYSCON->START[index].SRP;
    LPC_SYSCON->START[index].RSRPCLR = state[index];
  }

  while (current)
  {
    struct WakeupInt * const interrupt = *pointerListData(current);

    if (state[interrupt->index] & interrupt->mask)
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = pointerListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result startLogicHandlerAttach(uint8_t channel,
    struct WakeupInt *interrupt)
{
  if (!handler)
    handler = init(StartLogicHandler, 0);

  assert(handler);

  PointerList * const list = &handler->list;
  PointerListNode *current = pointerListFront(list);

  /* Check for duplicates */
  while (current)
  {
    struct WakeupInt * const entry = *pointerListData(current);

    if (entry->channel == channel)
      return E_BUSY;

    current = pointerListNext(current);
  }

  /* Add to list */
  return pointerListPushFront(list, interrupt) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void startLogicHandlerDetach(struct WakeupInt *interrupt)
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
static enum Result wakeupIntInit(void *object, const void *configBase)
{
  const struct WakeupIntConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  struct WakeupInt * const interrupt = object;
  const uint8_t channel = input.port * 12 + input.number;
  enum Result res;

  assert(config->event != PIN_TOGGLE);
  assert(pinValid(input));

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = startLogicHandlerAttach(channel, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->channel = channel;
  interrupt->index = channel >> 5;
  interrupt->mask = 1UL << (channel & 0x1F);

  /* Configure edge sensitivity options */
  switch (config->event)
  {
    case PIN_RISING:
      LPC_SYSCON->START[interrupt->index].APRP |= interrupt->mask;
      break;

    case PIN_FALLING:
      LPC_SYSCON->START[interrupt->index].APRP &= ~interrupt->mask;
      break;

    default:
      break;
  }

  /* Clear status flag */
  LPC_SYSCON->START[interrupt->index].RSRPCLR = interrupt->mask;

  /* Enable interrupt in NVIC, interrupt is masked by default */
  const IrqNumber irq = calcVector(interrupt->channel);

  irqSetPriority(irq, config->priority);
  irqEnable(irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_WAKEUPINT_NO_DEINIT
static void wakeupIntDeinit(void *object)
{
  struct WakeupInt * const interrupt = object;

  irqDisable(calcVector(interrupt->channel));
  LPC_SYSCON->START[interrupt->index].ERP &= ~interrupt->mask;

  startLogicHandlerDetach(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void wakeupIntEnable(void *object)
{
  struct WakeupInt * const interrupt = object;

  LPC_SYSCON->START[interrupt->index].RSRPCLR = interrupt->mask;
  LPC_SYSCON->START[interrupt->index].ERP |= interrupt->mask;
}
/*----------------------------------------------------------------------------*/
static void wakeupIntDisable(void *object)
{
  struct WakeupInt * const interrupt = object;
  LPC_SYSCON->START[interrupt->index].ERP &= ~interrupt->mask;
}
/*----------------------------------------------------------------------------*/
static void wakeupIntSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct WakeupInt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
