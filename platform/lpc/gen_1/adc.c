/*
 * adc.c
 * Copyright (C) 2016, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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
  const struct AdcPin * const pins = interface->pins;
  const LPC_ADC_Type * const reg = interface->base.reg;

  /* Read values and clear interrupt flags */
  for (size_t index = 0; index < interface->count; ++index)
    interface->buffer[index] = (uint16_t)reg->DR[pins[index].channel];

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

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
  assert(config->pins != NULL && *config->pins);
  assert(config->event < ADC_EVENT_END && config->event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct Adc * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->buffer =
      malloc((sizeof(uint16_t) + sizeof(struct AdcPin)) * count);
  if (interface->buffer == NULL)
    return E_MEMORY;
  interface->pins = (struct AdcPin *)(interface->buffer + count);

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->count = (uint8_t)count;
  interface->priority = config->priority;

  if (config->event == ADC_BURST)
    interface->base.control |= CR_BURST;
  else
    interface->base.control |= CR_START(config->event);

  /* Initialize input pins */
  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->base.control |= CR_SEL(mask);
  interface->event = interface->pins[count - 1].channel;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct Adc * const interface = object;

#  ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  if (adcGetInstance(interface->base.channel) == &interface->base)
#  endif
  {
    stopConversion(interface);
  }

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->buffer);

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
#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
      if (adcGetInstance(interface->base.channel) != &interface->base)
        return E_OK;
#endif

      return (reg->CR & (CR_START_MASK | CR_BURST)) ? E_BUSY : E_OK;

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
      return adcSetInstance(interface->base.channel, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, &interface->base, NULL);
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
