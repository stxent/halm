/*
 * pin_int.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_list.h>
#include <halm/platform/numicro/m03x/pin_defs.h>
#include <halm/platform/numicro/pin_int.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct PinIntHandlerConfig
{
  /** Mandatory: port identifier. */
  uint8_t port;
};

struct PinIntHandler
{
  struct Entity base;
  PointerList list;
};
/*----------------------------------------------------------------------------*/
static inline NM_GPIO_Type *calcPort(uint8_t);
static inline uint8_t calcPortGroup(uint8_t);
static void disableInterrupt(const struct PinInt *);
static void enableInterrupt(const struct PinInt *);
static void processInterrupt(uint8_t, uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result pinIntHandlerAttach(uint8_t, PinNumber, struct PinInt *);
static enum Result pinIntHandlerInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_PININT_NO_DEINIT
static void pinIntHandlerDetach(struct PinInt *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *, const void *);
static void pinIntEnable(void *);
static void pinIntDisable(void *);
static void pinIntSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NUMICRO_PININT_NO_DEINIT
static void pinIntDeinit(void *);
#else
#define pinIntDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PinIntHandler =
    &(const struct EntityClass){
    .size = sizeof(struct PinIntHandler),
    .init = pinIntHandlerInit,
    .deinit = deletedDestructorTrap
};

const struct InterruptClass * const PinInt = &(const struct InterruptClass){
    .size = sizeof(struct PinInt),
    .init = pinIntInit,
    .deinit = pinIntDeinit,

    .enable = pinIntEnable,
    .disable = pinIntDisable,
    .setCallback = pinIntSetCallback
};
/*----------------------------------------------------------------------------*/
static struct PinIntHandler *handlers[2] = {0};
/*----------------------------------------------------------------------------*/
static inline NM_GPIO_Type *calcPort(uint8_t port)
{
  return NM_GPIOA + port;
}
/*----------------------------------------------------------------------------*/
static inline uint8_t calcPortGroup(uint8_t port)
{
  return port >= PORT_C && port <= PORT_F;
}
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInt *interrupt)
{
  NM_GPIO_Type * const reg = calcPort(interrupt->port);
  const uint32_t mask = interrupt->mask;

  reg->INTEN &= ~(INTEN_FLIEN_GROUP(mask) | INTEN_RHIEN_GROUP(mask));
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInt *interrupt)
{
  NM_GPIO_Type * const reg = calcPort(interrupt->port);
  const enum PinEvent event = interrupt->event;
  const uint32_t mask = interrupt->mask;
  uint32_t inten = reg->INTEN;

  /* Clear pending interrupt flag */
  reg->INTSRC = mask;

  /* Configure sensitivity options */
  inten &= ~(INTEN_FLIEN_GROUP(mask) | INTEN_RHIEN_GROUP(mask));
  if (event == PIN_LOW || event == PIN_FALLING || event == PIN_TOGGLE)
    inten |= INTEN_FLIEN_GROUP(mask);
  if (event == PIN_HIGH || event == PIN_RISING || event == PIN_TOGGLE)
    inten |= INTEN_RHIEN_GROUP(mask);

  reg->INTEN = inten;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t group, uint8_t port)
{
  assert(handlers[group]);

  NM_GPIO_Type * const reg = calcPort(port);
  PointerList * const list = &handlers[group]->list;
  PointerListNode *current = pointerListFront(list);
  const uint32_t status = reg->INTSRC;

  /* Clear pending interrupt flags */
  reg->INTSRC = status;

  while (current)
  {
    struct PinInt * const interrupt = *pointerListData(current);

    if (status & interrupt->mask)
    {
      if (interrupt->callback)
        interrupt->callback(interrupt->callbackArgument);
    }

    current = pointerListNext(current);
  }
}
/*----------------------------------------------------------------------------*/
void GPIO_PAPBPGPH_ISR(void)
{
  if (NM_GPIOA->INTSRC)
    processInterrupt(0, PORT_A);
  if (NM_GPIOB->INTSRC)
    processInterrupt(0, PORT_B);
  if (NM_GPIOG->INTSRC)
    processInterrupt(0, PORT_G);
  if (NM_GPIOH->INTSRC)
    processInterrupt(0, PORT_H);
}
/*----------------------------------------------------------------------------*/
void GPIO_PCPDPEPF_ISR(void)
{
  if (NM_GPIOC->INTSRC)
    processInterrupt(1, PORT_C);
  if (NM_GPIOD->INTSRC)
    processInterrupt(1, PORT_D);
  if (NM_GPIOE->INTSRC)
    processInterrupt(1, PORT_E);
  if (NM_GPIOF->INTSRC)
    processInterrupt(1, PORT_F);
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntHandlerAttach(uint8_t group, PinNumber key,
    struct PinInt *interrupt)
{
  if (!handlers[group])
  {
    const struct PinIntHandlerConfig config = {group};
    handlers[group] = init(PinIntHandler, &config);
  }

  assert(handlers[group]);

  PointerList * const list = &handlers[group]->list;
  PointerListNode *current = pointerListFront(list);

  /* Check for duplicates */
  while (current)
  {
    struct PinInt * const entry = *pointerListData(current);

    if (entry->key == key)
      return E_BUSY;

    current = pointerListNext(current);
  }

  /* Add to list */
  return pointerListPushFront(list, interrupt) ? E_OK : E_MEMORY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_PININT_NO_DEINIT
static void pinIntHandlerDetach(struct PinInt *interrupt)
{
  const uint8_t group = calcPortGroup(interrupt->port);

  assert(pointerListFind(&handlers[group]->list, interrupt));
  pointerListErase(&handlers[group]->list, interrupt);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinIntHandlerInit(void *object, const void *configBase)
{
  const struct PinIntHandlerConfig * const config = configBase;
  struct PinIntHandler * const handler = object;

  if (calcPortGroup(config->port))
    irqEnable(GPIO_PCPDPEPF_IRQ);
  else
    irqEnable(GPIO_PAPBPGPH_IRQ);

  pointerListInit(&handler->list);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *object, const void *configBase)
{
  const struct PinIntConfig * const config = configBase;
  assert(config);
  assert(config->pull == PIN_NOPULL);
  assert(config->event != PIN_LOW && config->event != PIN_HIGH);

  const struct Pin input = pinInit(config->pin);
  assert(pinValid(input));

  struct PinInt * const interrupt = object;
  NM_GPIO_Type * const reg = calcPort(input.port);
  const uint8_t group = calcPortGroup(input.port);
  enum Result res;

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = pinIntHandlerAttach(group, config->pin, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->key = config->pin;
  interrupt->mask = 1 << input.number;
  interrupt->event = config->event;
  interrupt->port = input.port;
  interrupt->enabled = false;

  if (config->event == PIN_HIGH || config->event == PIN_LOW)
    reg->INTTYPE |= interrupt->mask;
  else
    reg->INTTYPE &= ~interrupt->mask;

  disableInterrupt(interrupt);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_PININT_NO_DEINIT
static void pinIntDeinit(void *object)
{
  disableInterrupt(object);
  pinIntHandlerDetach(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void pinIntEnable(void *object)
{
  struct PinInt * const interrupt = object;

  interrupt->enabled = true;

  if (interrupt->callback)
    enableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinIntDisable(void *object)
{
  struct PinInt * const interrupt = object;

  interrupt->enabled = false;
  disableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinIntSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;

  if (interrupt->enabled && callback)
    enableInterrupt(interrupt);
  else
    disableInterrupt(interrupt);
}
