/*
 * adc.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/adc.h>
#include <halm/platform/stm32/adc.h>
#include <halm/platform/stm32/gen_1/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void interruptHandler(void *);
static void startConversion(struct Adc *);
static void stopConversion(struct Adc *);

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
static void startCalibration(struct Adc *);
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
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
  STM_ADC_Type * const reg = interface->base.reg;

  const struct AdcPin *pin = interface->pins;
  uint16_t *buffer = interface->buffer;

  /* Clear pending data interrupt flags */
  reg->SR = 0;

  for (size_t index = 0; index < interface->count; ++index)
  {
    *buffer++ = (uint16_t)reg->JDR[index];
    ++pin;
  }

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
static void startCalibration(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  STM_ADC_Type * const reg = interface->base.reg;
  const bool enabled = (reg->CR2 & CR2_ADON) != 0;

  if (!enabled)
  {
    /* Enable ADC and wait for at least 2 APB clocks */
    reg->CR2 = CR2_ADON;
    udelay(10);
  }

  /* Start calibration */
  reg->CR2 |= CR2_CAL;

  if (!enabled)
  {
    while (reg->CR2 & CR2_CAL);
    reg->CR2 = 0;
  }
}
#endif
/*----------------------------------------------------------------------------*/
static void startConversion(struct Adc *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  STM_ADC_Type * const reg = interface->base.reg;
  uint32_t smpr[ARRAY_SIZE(reg->SMPR)] = {0};
  uint32_t jsqr = JSQR_JL(interface->count - 1);

  for (size_t sequence = 0; sequence < interface->count; ++sequence)
  {
    const struct AdcPin * const pin = &interface->pins[sequence];
    const unsigned int smprIndex = SMPR_SMP_REV_INDEX(pin->channel);
    const unsigned int smprOffset = SMPR_SMP_OFFSET(pin->channel);

    smpr[smprIndex] |= SMPR_SMP(smprOffset, interface->time);
    jsqr |= JSQR_JSQ(sequence, pin->channel);
  }

  /* Configure conversion sequence and sampling times */
  for (size_t index = 0; index < ARRAY_SIZE(smpr); ++index)
    reg->SMPR[index] = smpr[ARRAY_SIZE(smpr) - index - 1];
  reg->JSQR = jsqr;

  /* Configure the peripheral and start conversions */
  reg->CR1 = interface->control1;
  reg->CR2 = interface->control2;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Adc *interface)
{
  STM_ADC_Type * const reg = interface->base.reg;

  reg->CR2 = 0;
  reg->CR1 = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config != NULL);
  assert((config->accuracy - 6 <= 6 && config->accuracy % 2 == 0)
      || !config->accuracy);
  assert(config->pins != NULL);
  assert(config->event != ADC_INJ_SOFTWARE
      && config->event < ADC_INJ_EVENT_END);
  assert(config->sensitivity != INPUT_HIGH && config->sensitivity != INPUT_LOW);
  assert(config->time < ADC_SAMPLING_TIME_END);

  const struct AdcBaseConfig baseConfig = {
      .injected = config->event,
      .regular = ADC_SOFTWARE,
      .channel = config->channel,
      .shared = config->shared
  };
  struct Adc * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Initialize input pins */
  interface->count = calcPinCount(config->pins);
  assert(interface->count > 0 && interface->count <= 4);

  /* Allocate buffer for conversion results */
  interface->buffer = malloc(sizeof(uint16_t) * interface->count);
  if (interface->buffer == NULL)
    return E_MEMORY;
  memset(interface->buffer, 0, sizeof(uint16_t) * interface->count);

  /* Allocate buffer for pin descriptors */
  interface->pins = malloc(sizeof(struct AdcPin) * interface->count);
  if (interface->pins == NULL)
    return E_MEMORY;
  adcSetupPins(&interface->base, config->pins, interface->pins,
      interface->count);

  interface->control1 = CR1_JEOCIE | CR1_SCAN;

  if (config->accuracy)
    interface->control1 |= CR1_RES((12 - config->accuracy) >> 1);
  else
    interface->control1 |= CR1_RES(RES_12BIT);

  interface->control2 = CR2_ADON | CR2_ALIGN
      | (CR2_JEXTSEL(config->event) & CR2_JEXTSEL_MASK)
      | CR2_JEXTTRIG | CR2_JEXTEN(adcEncodeSensitivity(config->sensitivity));

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
  interface->control2 |= CR2_CAL;
#endif

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->time = config->time;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
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
    [[maybe_unused]] void *data)
{
  const struct Adc * const interface = object;
  const STM_ADC_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      if (adcGetInstance(interface->base.channel) == &interface->base)
        return (reg->CR2 & CR2_ADON) ? E_BUSY : E_OK;
      else
        return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    [[maybe_unused]] const void *data)
{
  struct Adc * const interface = object;

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      startCalibration(interface);
      return E_BUSY;

    default:
      break;
  }
#endif

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      startConversion(interface);
      return E_OK;

#ifdef CONFIG_PLATFORM_STM32_ADC_SHARED
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

  const IrqState state = irqSave();
  memcpy(buffer, interface->buffer, chunk);
  irqRestore(state);

  return chunk;
}
