/*
 * exti.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/exti.h>
#include <halm/platform/stm32/exti_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result extiInit(void *, const void *);
static void extiEnable(void *);
static void extiDisable(void *);
static void extiSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void extiDeinit(void *);
#else
#define extiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const Exti =
    &(const struct InterruptClass){
    .size = sizeof(struct Exti),
    .init = extiInit,
    .deinit = extiDeinit,

    .enable = extiEnable,
    .disable = extiDisable,
    .setCallback = extiSetCallback
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Exti * const interrupt = object;
  interrupt->callback(interrupt->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result extiInit(void *object, const void *configBase)
{
  const struct ExtiConfig * const config = configBase;
  assert(config != NULL);
  assert(config->event != INPUT_LOW && config->event != INPUT_HIGH);

  enum ExtiEvent channel = EXTI_EVENT_END;

  if (config->pin)
  {
    const struct Pin input = pinInit(config->pin);
    assert(pinValid(input));

    pinInput(input);
    pinSetPull(input, config->pull);

    channel = PIN_TO_OFFSET(config->pin);
  }
  else
  {
    assert(config->channel > EXTI_PIN15);
    channel = config->channel;
  }

  const struct ExtiBaseConfig baseConfig = {
      .pin = config->pin,
      .priority = config->priority,
      .channel = channel
  };
  struct Exti * const interrupt = object;

  /* Call base class constructor */
  const enum Result res = ExtiBase->init(interrupt, &baseConfig);
  if (res != E_OK)
    return res;

  interrupt->base.handler = interruptHandler;
  interrupt->callback = NULL;

  interrupt->mask = 1UL << interrupt->base.channel;
  interrupt->enabled = false;

  /* Configure edge sensitivity options */
  if (config->event == INPUT_RISING || config->event == INPUT_TOGGLE)
    STM_EXTI->RTSR |= interrupt->mask;
  if (config->event == INPUT_FALLING || config->event == INPUT_TOGGLE)
    STM_EXTI->FTSR |= interrupt->mask;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void extiDeinit(void *object)
{
  struct Exti * const interrupt = object;
  const uint32_t mask = ~interrupt->mask;

  /* Disable interrupts and events */
  STM_EXTI->IMR &= mask;
  STM_EXTI->EMR &= mask;

  ExtiBase->deinit(interrupt);
}
#endif
/*----------------------------------------------------------------------------*/
static void extiEnable(void *object)
{
  struct Exti * const interrupt = object;
  const uint32_t mask = interrupt->mask;

  interrupt->enabled = true;

  if (interrupt->callback != NULL)
  {
    STM_EXTI->EMR |= mask;
    STM_EXTI->IMR |= mask;
  }
}
/*----------------------------------------------------------------------------*/
static void extiDisable(void *object)
{
  struct Exti * const interrupt = object;
  const uint32_t mask = ~interrupt->mask;

  STM_EXTI->EMR &= mask;
  STM_EXTI->IMR &= mask;

  interrupt->enabled = false;
}
/*----------------------------------------------------------------------------*/
static void extiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Exti * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;

  if (interrupt->enabled && callback)
  {
    const uint32_t mask = interrupt->mask;

    STM_EXTI->EMR |= mask;
    STM_EXTI->IMR |= mask;
  }
  else
  {
    const uint32_t mask = ~interrupt->mask;

    STM_EXTI->EMR &= mask;
    STM_EXTI->IMR &= mask;
  }
}
