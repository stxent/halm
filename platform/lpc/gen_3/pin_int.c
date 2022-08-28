/*
 * pin_int.c
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_3/pin_defs.h>
#include <halm/platform/lpc/pin_int.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInt *);
static void enableInterrupt(const struct PinInt *);
static void interruptHandler(void *);
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
static void disableInterrupt(const struct PinInt *interrupt)
{
  irqDisable(interrupt->base.irq);
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInt *interrupt)
{
  /* Clear pending interrupt flags in the peripheral */
  LPC_GPIO_INT->IST = interrupt->mask;
  __dsb();

  /* Clear pending interrupts and enable further interrupts in the NVIC */
  irqClearPending(interrupt->base.irq);
  irqEnable(interrupt->base.irq);
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct PinInt * const interrupt = object;

  LPC_GPIO_INT->IST = interrupt->mask;

  if (interrupt->callback)
    interrupt->callback(interrupt->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *object, const void *configBase)
{
  const struct PinIntConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  assert(pinValid(input));

  const struct PinIntBaseConfig baseConfig = {
      .number = input.number,
      .port = input.port
  };
  struct PinInt * const interrupt = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PinIntBase->init(interrupt, &baseConfig)) != E_OK)
    return res;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->base.handler = interruptHandler;
  interrupt->callback = 0;

  interrupt->mask = 1 << interrupt->base.channel;
  interrupt->enabled = false;

  /* Configure interrupt as edge sensitive */
  LPC_GPIO_INT->ISEL &= ~interrupt->mask;

  /* Configure rising edge sensitivity */
  if (config->event == PIN_RISING || config->event == PIN_TOGGLE)
    LPC_GPIO_INT->SIENR = interrupt->mask;
  else
    LPC_GPIO_INT->CIENR = interrupt->mask;

  /* Configure falling edge sensitivity */
  if (config->event == PIN_FALLING || config->event == PIN_TOGGLE)
    LPC_GPIO_INT->SIENF = interrupt->mask;
  else
    LPC_GPIO_INT->CIENF = interrupt->mask;

  /* Configure interrupt priority, interrupt is disabled by default */
  irqSetPriority(interrupt->base.irq, config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
static void pinIntDeinit(void *object)
{
  struct PinInt * const interrupt = object;

  /* Disable channel interrupt in the NVIC */
  disableInterrupt(interrupt);

  /* Disable interrupt sources */
  LPC_GPIO_INT->CIENR = interrupt->mask;
  LPC_GPIO_INT->CIENF = interrupt->mask;

  PinIntBase->deinit(interrupt);
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
