/*
 * adc_burst.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <halm/platform/nxp/adc_burst.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcSetCallback(void *, void (*)(void *), void *);
static enum result adcGetParam(void *, enum IfParameter, void *);
static enum result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct AdcBurst),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcBurst = &adcTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcBurst * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;
  uint16_t *buffer = interface->buffer;

  /* Copy conversion result and increase position in buffer */
  *buffer++ = DR_RESULT_VALUE(reg->DR[interface->pin.channel]);
  interface->buffer = buffer;

  if (!--interface->left)
  {
    /* Stop automatic conversion */
    reg->CR &= ~(CR_BURST | CR_START_MASK);
    /* Disable interrupts */
    reg->INTEN = 0;
    irqDisable(unit->base.irq);

    adcUnitUnregister(unit);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
  else
  {
    reg->CR = reg->CR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configBase)
{
  const struct AdcBurstConfig * const config = configBase;
  assert(config);

  struct AdcBurst * const interface = object;

  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  /* Initialize input pin */
  adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);

  interface->callback = 0;
  interface->blocking = true;
  interface->buffer = 0;
  interface->left = 0;
  interface->unit = config->parent;
  /* Convert enumerator constant to register value */
  interface->event = config->event;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  const struct AdcBurst * const interface = object;

  adcReleasePin(interface->pin);
}
/*----------------------------------------------------------------------------*/
static enum result adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcBurst * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcGetParam(void *object, enum IfParameter parameter,
    void *data __attribute__((unused)))
{
  struct AdcBurst * const interface = object;

  switch (parameter)
  {
    case IF_STATUS:
      return interface->left ? E_BUSY : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSetParam(void *object, enum IfParameter parameter,
    const void *data __attribute__((unused)))
{
  struct AdcBurst * const interface = object;

  switch (parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct AdcBurst * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;
  const size_t samples = length / sizeof(uint16_t);

  if (!samples)
    return 0;

  if (adcUnitRegister(unit, interruptHandler, interface) != E_OK)
    return 0;

  interface->buffer = buffer;
  interface->left = samples;

  /* Clear pending interrupts */
  (void)reg->DR[interface->pin.channel];
  /* Enable interrupts for the channel */
  reg->INTEN = INTEN_AD(interface->pin.channel);
  irqEnable(unit->base.irq);

  /* Allow conversions */
  reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL_CHANNEL(interface->pin.channel);

  /* Start the conversion */
  if (interface->event == ADC_BURST)
    reg->CR |= CR_BURST;
  else
    reg->CR |= CR_START(interface->event);

  if (interface->blocking)
  {
    while (interface->left)
      barrier();
  }

  return samples * sizeof(uint16_t);
}
