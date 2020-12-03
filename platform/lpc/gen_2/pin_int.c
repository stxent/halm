/*
 * pin_int.c
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/lpc/gen_2/pin_int.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct PinIntHandlerConfig
{
  uint8_t channel;
};

struct PinIntHandler
{
  struct Entity base;
  struct PinInt *interrupts[12];
};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static inline IrqNumber calcVector(uint8_t);
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
static struct PinIntHandler *handlers[4] = {0};
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
static void disableInterrupt(const struct PinInt *interrupt)
{
  LPC_GPIO_Type * const reg = calcPort(interrupt->channel);
  reg->IE &= ~interrupt->mask;
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInt *interrupt)
{
  LPC_GPIO_Type * const reg = calcPort(interrupt->channel);

  reg->IC = interrupt->mask;
  reg->IE |= interrupt->mask;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  struct PinInt ** const interrupts = handlers[channel]->interrupts;
  LPC_GPIO_Type * const reg = calcPort(channel);
  uint32_t state = reg->MIS;

  /* Synchronizer logic causes a delay of 2 clocks */
  reg->IC = state;

  state = reverseBits32(state);

  do
  {
    const unsigned int index = countLeadingZeros32(state);
    struct PinInt * const interrupt = interrupts[index];

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
static enum Result pinIntHandlerAttach(uint8_t port, uint8_t number,
    struct PinInt *interrupt)
{
  if (!handlers[port])
  {
    const struct PinIntHandlerConfig handlerConfig = {
        .channel = port
    };
    handlers[port] = init(PinIntHandler, &handlerConfig);
  }

  struct PinIntHandler * const handler = handlers[port];
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
static enum Result pinIntHandlerInit(void *object, const void *configBase)
{
  struct PinIntHandler * const handler = object;
  const struct PinIntHandlerConfig * const config = configBase;

  for (size_t index = 0; index < ARRAY_SIZE(handler->interrupts); ++index)
    handler->interrupts[index] = 0;

  irqEnable(calcVector(config->channel));
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *object, const void *configBase)
{
  const struct PinIntConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  assert(pinValid(input));

  struct PinInt * const interrupt = object;
  enum Result res;

  /* Try to register pin interrupt in the interrupt handler */
  res = pinIntHandlerAttach(input.port, input.number, interrupt);
  if (res != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->mask = 1UL << input.number;
  interrupt->channel = input.port;
  interrupt->enabled = false;
  interrupt->event = config->event;

  LPC_GPIO_Type * const reg = calcPort(interrupt->channel);

  /* Configure interrupt as edge sensitive */
  reg->IS &= ~interrupt->mask;
  /* Configure edge sensitivity options */
  reg->IBE &= ~interrupt->mask;

  switch (interrupt->event)
  {
    case PIN_RISING:
      reg->IEV |= interrupt->mask;
      break;

    case PIN_FALLING:
      reg->IEV &= ~interrupt->mask;
      break;

    case PIN_TOGGLE:
      reg->IBE |= interrupt->mask;
      break;
  }
  /* Interrupt is disabled by default */

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
