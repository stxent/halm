/*
 * pin_int.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/lpc17xx/pin_defs.h>
#include <halm/platform/lpc/lpc17xx/system.h>
#include <halm/platform/lpc/lpc17xx/pin_int.h>
#include <xcore/accel.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct PinIntHandler
{
  struct Entity base;
  struct PinInt *interrupts[32];
};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInt *);
static void enableInterrupt(const struct PinInt *);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result pinIntHandlerAttach(uint8_t, uint8_t,
    struct PinInt *);
static enum Result pinIntHandlerInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
static void pinIntHandlerDetach(const struct PinInt *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *, const void *);
static void pinIntEnable(void *);
static void pinIntDisable(void *);
static void pinIntSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
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

const struct InterruptClass * const PinInt =
    &(const struct InterruptClass){
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
static void disableInterrupt(const struct PinInt *interrupt)
{
  const unsigned int channel = interrupt->channel;
  const uint32_t mask = ~interrupt->mask;

  LPC_GPIO_INT->PORT[channel].ENF &= ~mask;
  LPC_GPIO_INT->PORT[channel].ENR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInt *interrupt)
{
  const unsigned int channel = interrupt->channel;
  const enum PinEvent event = interrupt->event;

  /* Clear pending interrupt flag */
  LPC_GPIO_INT->PORT[channel].CLR = interrupt->mask;
  /* Configure edge sensitivity options */
  if (event != PIN_RISING)
    LPC_GPIO_INT->PORT[channel].ENF |= interrupt->mask;
  if (event != PIN_FALLING)
    LPC_GPIO_INT->PORT[channel].ENR |= interrupt->mask;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  assert(handlers[channel]);

  struct PinInt ** const interruptArray = handlers[channel]->interrupts;
  const uint32_t initialState = LPC_GPIO_INT->PORT[channel].STATR
      | LPC_GPIO_INT->PORT[channel].STATF;
  uint32_t state = reverseBits32(initialState);

  do
  {
    const unsigned int index = countLeadingZeros32(state);
    struct PinInt * const interrupt = interruptArray[index];

    state -= (1UL << 31) >> index;
    interrupt->callback(interrupt->callbackArgument);
  }
  while (state);

  LPC_GPIO_INT->PORT[channel].CLR = initialState;
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
static enum Result pinIntHandlerAttach(uint8_t channel,
    uint8_t number, struct PinInt *interrupt)
{
  if (!handlers[channel])
    handlers[channel] = init(PinIntHandler, 0);

  struct PinIntHandler * const handler = handlers[channel];

  assert(handler);

  if (!handler->interrupts[number])
  {
    handler->interrupts[number] = interrupt;
    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
static void pinIntHandlerDetach(const struct PinInt *interrupt)
{
  const unsigned int index = countLeadingZeros32(interrupt->mask);
  handlers[interrupt->channel]->interrupts[31 - index] = 0;
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinIntHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PinIntHandler * const handler = object;

  memset(handler->interrupts, 0, sizeof(handler->interrupts));

  sysClockControl(CLK_GPIOINT, DEFAULT_DIV);
  irqEnable(EINT3_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *object, const void *configBase)
{
  const struct PinIntConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  struct PinInt * const interrupt = object;
  enum Result res;

  assert(pinValid(input));

  /* External interrupt functionality is available only on two ports */
  assert(input.port == 0 || input.port == 2);

  /* Map ports 0 and 2 on channels 0 and 1 */
  interrupt->channel = input.port >> 1;

  /* Try to register pin interrupt in the interrupt handler */
  res = pinIntHandlerAttach(interrupt->channel, input.number, interrupt);
  if (res != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->enabled = false;
  interrupt->event = config->event;
  interrupt->mask = 1UL << input.number;

  disableInterrupt(interrupt);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
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
