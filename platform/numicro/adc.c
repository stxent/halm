/*
 * adc.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/adc.h>
#include <halm/platform/numicro/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void interruptHandler(void *);
static void startConversion(struct Adc *);
static void stopConversion(struct Adc *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
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
    .write = 0
};
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *pins)
{
  size_t index = 0;

  while (pins[index])
    ++index;

  return index;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Adc * const interface = object;
  NM_ADC_Type * const reg = interface->base.reg;
  const struct AdcPin *pins = interface->pins;
  uint16_t *buffer = interface->buffer;

  /* Clear pending interrupt flag */
  reg->ADSR0 = ADSR0_ADF;

  for (size_t index = 0; index < interface->count; ++index)
    *buffer++ = (uint16_t)reg->ADDR[pins[index].channel];

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  irqSetPriority(interface->base.irq, interface->priority);
  irqClearPending(interface->base.irq);
  irqEnable(interface->base.irq);

  /* Clear pending interrupt flag */
  reg->ADSR0 = ADSR0_ADF;
  /* Enable selected channels */
  reg->ADCHER = interface->enabled;
  /* Configure sample time extension */
  reg->ESMPCTL = interface->base.delay;
  /* Reconfigure peripheral and start the conversion */
  reg->ADCR = interface->base.control;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Adc *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  reg->ADCR = 0;
  irqDisable(interface->base.irq);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config);
  assert(config->pins);
  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .accuracy = config->accuracy,
      .channel = config->channel,
      .delay = config->delay,
      .shared = config->shared
  };
  struct Adc * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = AdcBase->init(interface, &baseConfig)) != E_OK)
    return res;

  /* Initialize input pins */
  interface->count = calcPinCount(config->pins);
  assert(interface->count > 0 && interface->count <= 32);

  /* Allocate buffer for conversion results */
  interface->buffer = malloc(sizeof(uint16_t) * interface->count);
  if (!interface->buffer)
    return E_MEMORY;
  memset(interface->buffer, 0, sizeof(uint16_t) * interface->count);

  /* Allocate buffer for pin descriptors */
  interface->pins = malloc(sizeof(struct AdcPin) * interface->count);
  if (!interface->pins)
    return E_MEMORY;
  interface->enabled = adcSetupPins(&interface->base, config->pins,
      interface->pins, interface->count);

  interface->base.handler = interruptHandler;
  interface->base.control |= ADCR_ADIE | ADCR_TRGEN
      | ADCR_ADMD(ADMD_SINGLE_SCAN) | ADCR_TRGS(config->event)
      | ADCR_TRGCOND(adcMakePinCondition(config->sensitivity));

  interface->callback = 0;
  interface->priority = config->priority;

  if (!config->shared)
    startConversion(interface);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct Adc * const interface = object;

  stopConversion(interface);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);

  free(interface->pins);
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
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
{
  struct Adc * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
    {
      const struct AdcBase * const instance =
          adcGetInstance(interface->base.channel);

      return (struct AdcBase *)interface == instance ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
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

#ifdef CONFIG_PLATFORM_NUMICRO_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, 0, object) ? E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, 0);
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
