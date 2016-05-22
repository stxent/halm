/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <memory.h>
#include <platform/nxp/lpc17xx/pin_defs.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct PinInterruptHandler
{
  struct Entity base;

  struct PinInterrupt *interrupts[32];
};
/*----------------------------------------------------------------------------*/
static void changeEnabledState(struct PinInterrupt *, bool);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerAttach(uint8_t, struct PinData,
    struct PinInterrupt *);
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
static struct PinInterruptHandler *handlers[2] = {0};
/*----------------------------------------------------------------------------*/
static void changeEnabledState(struct PinInterrupt *interrupt, bool state)
{
  const unsigned int channel = interrupt->channel;
  const uint32_t mask = 1 << interrupt->pin.offset;
  const enum pinEvent event = interrupt->event;

  if (state)
  {
    /* Clear pending interrupt flag */
    LPC_GPIO_INT->PORT[channel].CLR = mask;
    /* Configure edge sensitivity options */
    if (event != PIN_RISING)
      LPC_GPIO_INT->PORT[channel].ENF |= mask;
    if (event != PIN_FALLING)
      LPC_GPIO_INT->PORT[channel].ENR |= mask;
  }
  else
  {
    LPC_GPIO_INT->PORT[channel].ENF &= ~mask;
    LPC_GPIO_INT->PORT[channel].ENR &= ~mask;
  }
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  assert(handlers[channel]);

  struct PinInterrupt ** const interruptArray = handlers[channel]->interrupts;
  const uint32_t initialState = LPC_GPIO_INT->PORT[channel].STATR
      | LPC_GPIO_INT->PORT[channel].STATF;
  uint32_t state = reverseBits32(initialState);

  do
  {
    const unsigned int index = countLeadingZeros32(state);
    struct PinInterrupt * const interrupt = interruptArray[index];

    state -= BIT(31) >> index;
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
static enum result pinInterruptHandlerAttach(uint8_t channel,
    struct PinData pin, struct PinInterrupt *interrupt)
{
  if (!handlers[channel])
    handlers[channel] = init(PinInterruptHandler, 0);

  struct PinInterruptHandler * const handler = handlers[channel];

  assert(handler);

  if (!handler->interrupts[pin.offset])
  {
    handler->interrupts[pin.offset] = interrupt;
    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptHandlerDetach(const struct PinInterrupt *interrupt)
{
  const struct PinData pin = interrupt->pin;

  handlers[pin.port]->interrupts[pin.offset] = 0;
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PinInterruptHandler * const handler = object;

  memset(handler->interrupts, 0, sizeof(handler->interrupts));

  sysClockControl(CLK_GPIOINT, DEFAULT_DIV);
  irqEnable(EINT3_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *object, const void *configBase)
{
  const struct PinInterruptConfig * const config = configBase;
  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;
  enum result res;

  assert(pinValid(input));

  /* External interrupt functionality is available only on two ports */
  assert(input.data.port == 0 || input.data.port == 2);

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
  interrupt->enabled = false;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  changeEnabledState(interrupt, false);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  struct PinInterrupt * const interrupt = object;

  changeEnabledState(interrupt, false);
  pinInterruptHandlerDetach(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
  changeEnabledState(interrupt, callback != 0 && interrupt->enabled);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptSetEnabled(void *object, bool state)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->enabled = state;
  changeEnabledState(interrupt, state && interrupt->callback != 0);
}
