/*
 * adc_bus.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/adc_bus.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static size_t setupChannels(struct AdcBus *, const PinNumber *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, enum IfParameter, void *);
static enum Result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
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

  /* Further conversions should be stopped immediately after entering the ISR */
  if (!--interface->cycles)
  {
    /* Stop further conversions */
    reg->CR = 0;
    /* Disable interrupts */
    reg->INTEN = 0;
    irqDisable(interface->base.irq);
  }

  /* Read values and clear interrupt flags */
  for (size_t i = 0; i < interface->count; ++i)
    interface->buffer[i] = DR_RESULT_VALUE(reg->DR[interface->pins[i].channel]);

  if (!interface->cycles)
  {
    adcResetInstance(interface->base.channel);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static size_t setupChannels(struct AdcBus *interface, const PinNumber *pins)
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
      .channel = config->channel
  };
  struct AdcBus * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    interface->base.handler = interruptHandler;
    interface->callback = 0;
    interface->buffer = 0;
    interface->cycles = 0;
    interface->priority = config->priority;
    interface->blocking = true;

    if (config->event == ADC_BURST)
      interface->base.control |= CR_BURST;
    else
      interface->base.control |= CR_START(config->event);

    /* Initialize input pins */
    interface->count = setupChannels(interface, config->pins);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBus * const interface = object;

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->pins);

  if (AdcBase->deinit)
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
static enum Result adcGetParam(void *object, enum IfParameter parameter,
    void *data __attribute__((unused)))
{
  struct AdcBus * const interface = object;

  switch (parameter)
  {
    case IF_STATUS:
      return interface->buffer ? E_BUSY : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, enum IfParameter parameter,
    const void *data __attribute__((unused)))
{
  struct AdcBus * const interface = object;

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
  struct AdcBus * const interface = object;
  LPC_ADC_Type * const reg = interface->base.reg;
  const size_t cycles = length / (interface->count * sizeof(uint16_t));

  if (!cycles)
    return 0;
  if (!adcSetInstance(interface->base.channel, &interface->base))
    return 0;

  interface->buffer = buffer;
  interface->cycles = cycles;

  /* Clear pending interrupts */
  (void)reg->DR[interface->event];

  /* Enable interrupts for the channel with the highest number */
  reg->INTEN = INTEN_AD(interface->event);
  irqSetPriority(interface->base.irq, interface->priority);
  irqEnable(interface->base.irq);

  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;

  if (interface->blocking)
  {
    while (interface->cycles)
      barrier();
  }

  return cycles * interface->count * sizeof(uint16_t);
}
