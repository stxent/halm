/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc17xx/pin_defs.h>
#include <halm/platform/nxp/lpc17xx/system.h>
#include <halm/platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct PinInterruptHandler
{
  struct Entity base;
  struct PinInterrupt *interrupts[32];
};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInterrupt *);
static void enableInterrupt(const struct PinInterrupt *);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptHandlerAttach(uint8_t, struct PinData,
    struct PinInterrupt *);
static enum Result pinInterruptHandlerInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_PININT_NO_DEINIT
static void pinInterruptHandlerDetach(const struct PinInterrupt *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptInit(void *, const void *);
static void pinInterruptEnable(void *);
static void pinInterruptDisable(void *);
static void pinInterruptSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NXP_PININT_NO_DEINIT
static void pinInterruptDeinit(void *);
#else
#define pinInterruptDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PinInterruptHandler =
    &(const struct EntityClass){
    .size = sizeof(struct PinInterruptHandler),
    .init = pinInterruptHandlerInit,
    .deinit = deletedDestructorTrap
};

const struct InterruptClass * const PinInterrupt =
    &(const struct InterruptClass){
    .size = sizeof(struct PinInterrupt),
    .init = pinInterruptInit,
    .deinit = pinInterruptDeinit,

    .enable = pinInterruptEnable,
    .disable = pinInterruptDisable,
    .setCallback = pinInterruptSetCallback
};
/*----------------------------------------------------------------------------*/
static struct PinInterruptHandler *handlers[2] = {0};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInterrupt *interrupt)
{
  const unsigned int channel = interrupt->channel;
  const uint32_t mask = 1UL << interrupt->pin.offset;

  LPC_GPIO_INT->PORT[channel].ENF &= ~mask;
  LPC_GPIO_INT->PORT[channel].ENR &= ~mask;
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInterrupt *interrupt)
{
  const unsigned int channel = interrupt->channel;
  const uint32_t mask = 1UL << interrupt->pin.offset;
  const enum PinEvent event = interrupt->event;

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
  assert(handlers[channel]);

  struct PinInterrupt ** const interruptArray = handlers[channel]->interrupts;
  const uint32_t initialState = LPC_GPIO_INT->PORT[channel].STATR
      | LPC_GPIO_INT->PORT[channel].STATF;
  uint32_t state = reverseBits32(initialState);

  do
  {
    const unsigned int index = countLeadingZeros32(state);
    struct PinInterrupt * const interrupt = interruptArray[index];

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
static enum Result pinInterruptHandlerAttach(uint8_t channel,
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
#ifndef CONFIG_PLATFORM_NXP_PININT_NO_DEINIT
static void pinInterruptHandlerDetach(const struct PinInterrupt *interrupt)
{
  const struct PinData pin = interrupt->pin;

  handlers[pin.port]->interrupts[pin.offset] = 0;
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct PinInterruptHandler * const handler = object;

  memset(handler->interrupts, 0, sizeof(handler->interrupts));

  sysClockControl(CLK_GPIOINT, DEFAULT_DIV);
  irqEnable(EINT3_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptInit(void *object, const void *configBase)
{
  const struct PinInterruptConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;
  enum Result res;

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

  disableInterrupt(interrupt);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_PININT_NO_DEINIT
static void pinInterruptDeinit(void *object)
{
  disableInterrupt(object);
  pinInterruptHandlerDetach(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void pinInterruptEnable(void *object)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->enabled = true;

  if (interrupt->callback)
    enableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDisable(void *object)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->enabled = false;
  disableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;

  if (interrupt->enabled && callback)
    enableInterrupt(interrupt);
  else
    disableInterrupt(interrupt);
}
