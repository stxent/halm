/*
 * adc_bus.c
 * Copyright (C) 2016, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_bus.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t setupPins(struct AdcBus *, const PinNumber *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcBus = &(const struct InterfaceClass){
    .size = sizeof(struct AdcBus),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcBus * const interface = object;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->CR = 0;
  /* Disable interrupts */
  reg->INTEN = 0;

  /* Read values and clear interrupt flags */
  for (size_t i = 0; i < interface->count; ++i)
    interface->buffer[i] = DR_RESULT_VALUE(reg->DR[interface->pins[i].channel]);

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static size_t setupPins(struct AdcBus *interface, const PinNumber *pins)
{
  unsigned int event = 0;
  unsigned int mask = 0;
  size_t index = 0;

  while (pins[index])
  {
    assert(index < ARRAY_SIZE(interface->pins));
    interface->pins[index] = adcConfigPin(&interface->base, pins[index]);

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    const unsigned int channel = interface->pins[index].channel;
    assert(!(mask >> channel));

    if (channel > event)
      event = channel;
    mask |= 1 << channel;

    ++index;
  }

  interface->base.control |= CR_SEL(mask);
  interface->event = event;

  return index;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBusConfig * const config = configBase;
  assert(config);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcBus * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    interface->base.handler = interruptHandler;
    interface->callback = 0;
    interface->buffer = 0;
    interface->priority = config->priority;
    interface->blocking = true;

    if (config->event == ADC_BURST)
      interface->base.control |= CR_BURST;
    else
      interface->base.control |= CR_START(config->event);

    /* Initialize input pins */
    interface->count = setupPins(interface, config->pins);

    if (!config->shared)
    {
      irqSetPriority(interface->base.irq, interface->priority);
      irqEnable(interface->base.irq);
    }
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBus * const interface = object;

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);

  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcBus * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
{
  struct AdcBus * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return interface->buffer ? E_BUSY : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct AdcBus * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
    case IF_ACQUIRE:
      if (adcSetInstance(interface->base.channel, 0, object))
      {
        irqSetPriority(interface->base.irq, interface->priority);
        irqEnable(interface->base.irq);
        return E_BUSY;
      }
      else
        return E_BUSY;

    case IF_RELEASE:
      if (adcSetInstance(interface->base.channel, object, 0))
        irqDisable(interface->base.irq);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct AdcBus * const interface = object;
  LPC_ADC_Type * const reg = interface->base.reg;

  assert(length >= interface->count * sizeof(uint16_t));

  interface->buffer = buffer;

  /* Clear pending interrupt */
  (void)reg->DR[interface->event];
  /* Enable interrupt for the channel with the highest number */
  reg->INTEN = INTEN_AD(interface->event);
  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;

  if (interface->blocking)
  {
    while (reg->INTEN & INTEN_AD_MASK);
  }

  return interface->count * sizeof(uint16_t);
}
