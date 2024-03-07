/*
 * adc.c
 * Copyright (C) 2016, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t setupPins(struct Adc *, const PinNumber *);
static void startConversion(struct Adc *);
static void stopConversion(struct Adc *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Adc = &(const struct InterfaceClass){
    .size = sizeof(struct Adc),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = NULL
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Adc * const interface = object;

  /* Read values and clear interrupt flags */
  for (size_t index = 0; index < interface->count; ++index)
    interface->buffer[index] = (uint16_t)(*interface->dr[index]);

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static size_t setupPins(struct Adc *interface, const PinNumber *pins)
{
  LPC_ADC_Type * const reg = interface->base.reg;
  size_t index = 0;
  uint32_t enabled = 0;
  unsigned int event = 0;

  while (pins[index])
  {
    assert(index < ARRAY_SIZE(interface->pins));
    const struct AdcPin pin = adcConfigPin(&interface->base, pins[index]);

    interface->pins[index] = pin;
    interface->dr[index] = (const uint32_t *)&reg->DR[pin.channel];

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    const unsigned int channel = interface->pins[index].channel;
    assert(!(enabled >> channel));

    event = channel;
    enabled |= 1 << channel;
    ++index;
  }

  assert(enabled != 0);
  interface->base.control |= CR_SEL(enabled);
  interface->event = event;

  return index;
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  irqSetPriority(interface->base.irq, interface->priority);
  irqClearPending(interface->base.irq);
  irqEnable(interface->base.irq);

  /* Clear pending interrupt */
  (void)reg->DR[interface->event];
  /* Enable interrupt for the channel with the highest number */
  reg->INTEN = INTEN_AD(interface->event);
  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Adc *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->CR = 0;
  /* Disable interrupts */
  reg->INTEN = 0;

  irqDisable(interface->base.irq);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL);
  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct Adc * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->priority = config->priority;
  memset(interface->buffer, 0, sizeof(interface->buffer));

  if (config->event == ADC_BURST)
    interface->base.control |= CR_BURST;
  else
    interface->base.control |= CR_START(config->event);

  /* Initialize input pins */
  interface->count = setupPins(interface, config->pins);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct Adc * const interface = object;

  stopConversion(interface);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Adc * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter, void *)
{
  const struct Adc * const interface = object;
  const LPC_ADC_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      if (adcGetInstance(interface->base.channel) == &interface->base)
        return (reg->CR & (CR_START_MASK | CR_BURST)) ? E_BUSY : E_OK;
      else
        return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct Adc * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      startConversion(interface);
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, NULL, object) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, NULL);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct Adc * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  irqDisable(interface->base.irq);
  memcpy(buffer, interface->buffer, chunk);
  irqEnable(interface->base.irq);

  return chunk;
}
