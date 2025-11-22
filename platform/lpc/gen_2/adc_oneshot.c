/*
 * adc_oneshot.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/platform/lpc/adc_oneshot.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static void makeChannelConversion(struct AdcOneShot *, uint16_t *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcOneShot = &(const struct InterfaceClass){
    .size = sizeof(struct AdcOneShot),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = NULL,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = NULL
};
/*----------------------------------------------------------------------------*/
static void makeChannelConversion(struct AdcOneShot *interface,
    uint16_t *output)
{
  /* ADC should be locked to avoid simultaneous access */
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  const struct AdcPin * const pins = interface->pins;
  const unsigned int number = interface->base.sequence & 1;
  const uint32_t eoc = FLAGS_SEQ_INT(number);
  LPC_ADC_Type * const reg = interface->base.reg;
  IrqState state;

  /* Configure the sequence, but don't enable it */
  reg->SEQ_CTRL[number] = interface->control;

  /* Clear pending interrupt flag */
  reg->FLAGS = eoc;

  /* Enable End-of-Sequence interrupt */
  state = irqSave();
  reg->INTEN |= INTEN_SEQ_INTEN(number);
  irqRestore(state);

  /* Start single sequence conversion */
  reg->SEQ_CTRL[number] |= SEQ_CTRL_START | SEQ_CTRL_SEQ_ENA;
  while (!(reg->FLAGS & eoc));
  reg->SEQ_CTRL[number] &= ~SEQ_CTRL_SEQ_ENA;

  /* Disable End-of-Sequence interrupt */
  state = irqSave();
  reg->INTEN &= ~INTEN_SEQ_INTEN(number);
  irqRestore(state);

  for (size_t index = 0; index < interface->count; ++index)
    output[index] = (uint16_t)reg->DAT[pins[index].channel];
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcOneShotConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);
  assert(!config->preemption || (config->sequence & 1) == 0);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .sequence = config->sequence,
      .accuracy = 0,
      .shared = config->shared
  };
  struct AdcOneShot * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->pins = malloc(sizeof(struct AdcPin) * count);
  if (interface->pins == NULL)
    return E_MEMORY;

  interface->count = (uint8_t)count;

  /* Calculate Control register value */
  interface->control = SEQ_CTRL_MODE;
  if (config->preemption)
    interface->control |= SEQ_CTRL_LOWPRIO;

  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->control |= SEQ_CTRL_CHANNELS(mask);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcOneShot * const interface = object;

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->pins);

  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter, void *)
{
  const struct AdcOneShot * const interface = object;
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
  struct AdcOneShot * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      adcStartCalibration(&interface->base);
      return E_OK;

    default:
      break;
  }

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  switch ((enum IfParameter)parameter)
  {
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.sequence, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.sequence, &interface->base, NULL);
      return E_OK;

    default:
      break;
  }
#endif

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer,
    [[maybe_unused]] size_t length)
{
  struct AdcOneShot * const interface = object;

  /* Ensure that the buffer has enough space */
  assert(length >= interface->count * sizeof(uint16_t));

  makeChannelConversion(interface, buffer);
  return interface->count * sizeof(uint16_t);
}
