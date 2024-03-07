/*
 * adc_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/adc.h>
#include <halm/platform/stm32/adc_dma.h>
#include <halm/platform/stm32/gen_2/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void dmaHandler(void *);
static bool dmaSetup(struct AdcDma *, const struct AdcDmaConfig *);
static bool startConversion(struct AdcDma *);
static void stopConversion(struct AdcDma *);

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
static void startCalibration(struct AdcDma *);
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
const struct InterfaceClass * const AdcDma = &(const struct InterfaceClass){
    .size = sizeof(struct AdcDma),
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
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config)
{
  static const struct DmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = true
      }
  };

  interface->dma = adcMakeCircularDma(config->channel, config->dma,
      DMA_PRIORITY_LOW, true);

  if (interface->dma != NULL)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void startCalibration(struct AdcDma *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  STM_ADC_Type * const reg = interface->base.reg;

  /* ADC should be disabled during calibration */
  assert(!(reg->CR & CR_ADEN));

  /* Start calibration and wait for completion */
  reg->CR |= CR_ADCAL;
  while (reg->CR & CR_ADCAL);
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct AdcDma *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  STM_ADC_Type * const reg = interface->base.reg;

  dmaAppend(interface->dma, interface->buffer, (const void *)&reg->DR,
      interface->count);
  if (dmaEnable(interface->dma) != E_OK)
    return false;

  uint32_t enabled = 0;

  for (size_t index = 0; index < interface->count; ++index)
    enabled |= CHSELR_CHSEL(interface->pins[index].channel);

  /* Configure channels and sampling time */
  reg->SMPR = SMPR_SMP(interface->time);
  reg->CHSELR = enabled;

  /* Configure the peripheral and start conversions */
  reg->CFGR1 = interface->config;
  reg->CR |= CR_ADEN | CR_ADSTART;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct AdcDma *interface)
{
  STM_ADC_Type * const reg = interface->base.reg;

  if (reg->CR & CR_ADSTART)
  {
    reg->CR |= CR_ADSTP;
    while (reg->CR & CR_ADSTP);
  }

  reg->CR |= CR_ADDIS;
  dmaDisable(interface->dma);
  dmaClear(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config != NULL);
  assert((config->accuracy - 6 <= 6 && config->accuracy % 2 == 0)
      || !config->accuracy);
  assert(config->pins != NULL);
  assert(config->event != ADC_SOFTWARE && config->event < ADC_EVENT_END);
  assert(config->sensitivity != INPUT_HIGH && config->sensitivity != INPUT_LOW);
  assert(config->time < ADC_SAMPLING_TIME_END);

  const struct AdcBaseConfig baseConfig = {
      .injected = ADC_INJ_SOFTWARE,
      .regular = config->event,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Initialize input pins */
  interface->count = calcPinCount(config->pins);
  assert(interface->count > 0 && interface->count < 32);

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

  interface->config = CFGR1_DMAEN | CFGR1_DMACFG | CFGR1_ALIGN
      | CFGR1_EXTSEL(config->event)
      | CFGR1_EXTEN(adcEncodeSensitivity(config->sensitivity));

  if (config->accuracy)
    interface->config |= CFGR1_RES((12 - config->accuracy) >> 1);
  else
    interface->config |= CFGR1_RES(RES_12BIT);

  interface->callback = NULL;
  interface->time = config->time;

  if (!dmaSetup(interface, config))
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

  stopConversion(interface);
  deinit(interface->dma);

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
  struct AdcDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter, void *)
{
  const struct AdcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      if (adcGetInstance(interface->base.channel) == &interface->base)
        return dmaStatus(interface->dma);
      else
        return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct AdcDma * const interface = object;

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
      return startConversion(interface) ? E_OK : E_ERROR;

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
  struct AdcDma * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  memcpy(buffer, interface->buffer, chunk);
  return chunk;
}
