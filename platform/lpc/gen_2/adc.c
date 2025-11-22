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
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Clear pending interrupt flag */
  reg->FLAGS = FLAGS_SEQ_INT(interface->base.sequence & 1);

  for (size_t index = 0; index < interface->count; ++index)
    interface->buffer[index] = (uint16_t)reg->DAT[pins[index].channel];

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  const unsigned int number = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Configure the sequence, but don't enable it */
  reg->SEQ_CTRL[number] = interface->control;

  /* Clear pending interrupt flag */
  reg->FLAGS = FLAGS_SEQ_INT(number);

  /* Enable End-of-Sequence interrupt */
  const IrqState state = irqSave();
  reg->INTEN |= INTEN_SEQ_INTEN(number);
  irqRestore(state);

  /* Configure and enable NVIC interrupt */
  irqSetPriority(interface->base.irq.seq, interface->priority);
  irqClearPending(interface->base.irq.seq);
  irqEnable(interface->base.irq.seq);

  /* Enable sequence */
  reg->SEQ_CTRL[number] |= SEQ_CTRL_SEQ_ENA;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Adc *interface)
{
  const unsigned int number = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->SEQ_CTRL[number] &= ~SEQ_CTRL_SEQ_ENA;

  /* Disable NVIC interrupt */
  irqDisable(interface->base.irq.seq);

  /* Disable End-of-Sequence interrupt */
  const IrqState state = irqSave();
  reg->INTEN &= ~INTEN_SEQ_INTEN(number);
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);
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

  interface->control = SEQ_CTRL_MODE;
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
  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->control |= SEQ_CTRL_CHANNELS(mask);
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
      adcStartCalibration(&interface->base);
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
    *result++ = interface->buffer[index++];
  irqEnable(interface->base.irq.seq);

  return count * sizeof(uint16_t);
}
