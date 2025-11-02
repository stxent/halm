/*
 * adc.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/platform/lpc/adc.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static void setupPins(struct Adc *, const PinNumber *, size_t);
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
  struct AdcChannelTuple *entry = interface->channels;
  const struct AdcChannelTuple * const end = entry + interface->count;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Clear pending interrupt flag */
  reg->FLAGS = (interface->base.sequence & 1) ? FLAGS_SEQB_INT : FLAGS_SEQA_INT;

  do
  {
    entry->data = (uint16_t)reg->DAT[entry->pin.channel];
  }
  while (++entry != end);

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setupPins(struct Adc *interface, const PinNumber *pins,
    size_t count)
{
  uint32_t enabled = 0;

  for (size_t index = 0; index < count; ++index)
  {
    const struct AdcPin pin = adcConfigPin(&interface->base, pins[index]);

    interface->channels[index].data = 0;
    interface->channels[index].pin = pin;

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    assert(!(enabled >> pin.channel));

    enabled |= 1 << pin.channel;
  }

  interface->control |= SEQ_CTRL_CHANNELS(enabled);
}
/*----------------------------------------------------------------------------*/
static void startCalibration(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  LPC_ADC_Type * const reg = interface->base.reg;

  /* Reconfigure ADC clock */
  adcEnterCalibrationMode(&interface->base);

  /* Start calibration */
  reg->CTRL |= CTRL_LPWRMODE;
  reg->CTRL |= CTRL_CALMODE;
  reg->CTRL &= ~CTRL_LPWRMODE;
  while (reg->CTRL & CTRL_CALMODE);

  /* Restore configuration */
  reg->CTRL = interface->base.control;
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  const unsigned int sequenceB = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Clear pending interrupt flag */
  reg->FLAGS = sequenceB ? FLAGS_SEQB_INT : FLAGS_SEQA_INT;

  /* Configure and enable sequence interrupt */
  irqSetPriority(interface->base.irq.seq, interface->priority);
  irqClearPending(interface->base.irq.seq);
  irqEnable(interface->base.irq.seq);

  /* Enable end-of-sequence interrupt */
  const IrqState state = irqSave();
  reg->INTEN |= sequenceB ? INTEN_SEQB_INTEN : INTEN_SEQA_INTEN;
  irqRestore(state);

  reg->SEQ_CTRL[sequenceB] = interface->control;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Adc *interface)
{
  const unsigned int sequenceB = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->SEQ_CTRL[sequenceB] = 0;

  /* Disable interrupt */
  const IrqState state = irqSave();
  reg->INTEN &= sequenceB ? ~INTEN_SEQB_INTEN : ~INTEN_SEQA_INTEN;
  irqRestore(state);

  irqDisable(interface->base.irq.seq);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config != NULL && config->pins != NULL);
  assert(config->event < ADC_EVENT_END);
  assert(!config->preemption || (config->sequence & 1) == 0);
  assert(config->sensitivity <= INPUT_FALLING);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .sequence = config->sequence,
      .accuracy = config->accuracy,
      .shared = config->shared
  };
  struct Adc * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;
  if (!count)
    return E_VALUE;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->priority = config->priority;

  interface->control = SEQ_CTRL_MODE | SEQ_CTRL_SEQ_ENA;
  if (config->event == ADC_BURST)
    interface->control |= SEQ_CTRL_BURST;
  else
    interface->control |= SEQ_CTRL_TRIGGER(config->event);
  if (config->preemption)
    interface->control |= SEQ_CTRL_LOWPRIO;
  if (config->sensitivity == INPUT_RISING)
    interface->control |= SEQ_CTRL_TRIGPOL;
  if (config->singlestep)
    interface->control |= SEQ_CTRL_SINGLESTEP;

  /* Initialize input pins */
  interface->channels = malloc(sizeof(struct AdcChannelTuple) * count);
  if (interface->channels == NULL)
    return E_MEMORY;

  interface->count = (uint8_t)count;
  setupPins(interface, config->pins, count);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct Adc * const interface = object;

#  ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  if (adcGetInstance(interface->base.sequence) == &interface->base)
#  endif
  {
    stopConversion(interface);
  }

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->channels[index].pin);

  free(interface->channels);
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
      if (adcGetInstance(interface->base.sequence) != &interface->base)
        return E_OK;
#endif

      return (reg->SEQ_CTRL[interface->base.sequence & 1] & SEQ_CTRL_SEQ_ENA) ?
          E_BUSY : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct Adc * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      startCalibration(interface);
      return E_OK;

    default:
      break;
  }

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
      return adcSetInstance(interface->base.sequence, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.sequence, &interface->base, NULL);
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
  size_t count = interface->count * sizeof(uint16_t);
  size_t index = 0;
  uint16_t *result = buffer;

  if (length < count)
    count = length;
  count >>= 1;

  irqDisable(interface->base.irq.seq);
  while (index < count)
    *result++ = interface->channels[index++].data;
  irqEnable(interface->base.irq.seq);

  return count * sizeof(uint16_t);
}
