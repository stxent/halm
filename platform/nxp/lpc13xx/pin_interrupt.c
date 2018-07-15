/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <xcore/asm.h>
#include <halm/platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
struct PinInterruptHandlerConfig
{
  uint8_t channel;
};

struct PinInterruptHandler
{
  struct Entity base;
  struct PinInterrupt *interrupts[12];
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static inline IrqNumber calcVector(uint8_t);
static void disableInterrupt(const struct PinInterrupt *);
static void enableInterrupt(const struct PinInterrupt *);
static void processInterrupt(uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptHandlerAttach(struct PinData,
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
static struct PinInterruptHandler *handlers[4] = {0};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t port)
{
  return (LPC_GPIO_Type *)((uintptr_t)LPC_GPIO0 +
      ((uintptr_t)LPC_GPIO1 - (uintptr_t)LPC_GPIO0) * port);
}
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(uint8_t port)
{
  return PIOINT0_IRQ - port;
}
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInterrupt *interrupt)
{
  const struct PinData data = interrupt->pin;
  LPC_GPIO_Type * const reg = calcPort(data.port);

  reg->IE &= ~(1UL << data.offset);
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInterrupt *interrupt)
{
  const struct PinData data = interrupt->pin;
  const uint32_t mask = 1UL << data.offset;
  LPC_GPIO_Type * const reg = calcPort(data.port);

  reg->IC = mask;
  reg->IE |= mask;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  struct PinInterrupt ** const interruptArray = handlers[channel]->interrupts;
  LPC_GPIO_Type * const reg = calcPort(channel);
  uint32_t state = reg->MIS;

  /* Synchronizer logic causes a delay of 2 clocks */
  reg->IC = state;

  state = reverseBits32(state);

  do
  {
    const unsigned int index = countLeadingZeros32(state);
    struct PinInterrupt * const interrupt = interruptArray[index];

    state -= (1UL << 31) >> index;
    interrupt->callback(interrupt->callbackArgument);
  }
  while (state);
}
/*----------------------------------------------------------------------------*/
void PIO0_ISR(void)
{
  processInterrupt(0);
}
/*----------------------------------------------------------------------------*/
void PIO1_ISR(void)
{
  processInterrupt(1);
}
/*----------------------------------------------------------------------------*/
void PIO2_ISR(void)
{
  processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
void PIO3_ISR(void)
{
  processInterrupt(3);
}
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptHandlerAttach(struct PinData pin,
    struct PinInterrupt *interrupt)
{
  if (!handlers[pin.port])
  {
    const struct PinInterruptHandlerConfig handlerConfig = {
        .channel = pin.port
    };

    handlers[pin.port] = init(PinInterruptHandler, &handlerConfig);
  }

  struct PinInterruptHandler * const handler = handlers[pin.port];

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
static enum Result pinInterruptHandlerInit(void *object, const void *configBase)
{
  struct PinInterruptHandler * const handler = object;
  const struct PinInterruptHandlerConfig * const config = configBase;

  memset(handler->interrupts, 0, sizeof(handler->interrupts));
  irqEnable(calcVector(config->channel));

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

  /* Try to register pin interrupt in the interrupt handler */
  if ((res = pinInterruptHandlerAttach(input.data, interrupt)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->channel = 0; /* Channel field is left unused */
  interrupt->enabled = false;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  LPC_GPIO_Type * const reg = calcPort(interrupt->pin.port);
  const uint32_t mask = 1UL << interrupt->pin.offset;

  /* Configure interrupt as edge sensitive */
  reg->IS &= ~mask;
  /* Configure edge sensitivity options */
  reg->IBE &= ~mask;

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

  /* Interrupt is disabled by default */
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
