/*
 * adc_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/generic/adc.h>
#include <halm/platform/stm32/adc_dma.h>
#include <halm/platform/stm32/gen_1/adc_defs.h>
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
#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
static void startCalibration(struct AdcDma *interface)
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
static bool startConversion(struct AdcDma *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  STM_ADC_Type * const reg = interface->base.reg;

  dmaAppend(interface->dma, interface->buffer, (const void *)&reg->DR,
      interface->count);
  if (dmaEnable(interface->dma) != E_OK)
    return false;

  uint32_t smpr[ARRAY_SIZE(reg->SMPR)] = {0};
  uint32_t sqr[ARRAY_SIZE(reg->SQR)] = {0};

  sqr[ARRAY_SIZE(sqr) - 1] = SQR1_L(interface->count - 1);

  for (unsigned int sqrIndex = 0; sqrIndex < interface->count; ++sqrIndex)
  {
    for (unsigned int sqrOffset = 0; sqrOffset < 6; ++sqrOffset)
    {
      const size_t index = sqrIndex * 6 + sqrOffset;

      if (index >= interface->count)
        break;

      const struct AdcPin * const pin = &interface->pins[index];
      const unsigned int smprIndex = SMPR_SMP_REV_INDEX(pin->channel);
      const unsigned int smprOffset = SMPR_SMP_OFFSET(pin->channel);

      smpr[smprIndex] |= SMPR_SMP(smprOffset, interface->time);
      sqr[sqrIndex] |= SQR_SQ(sqrOffset, pin->channel);
    }
  }

  /* Configure conversion sequence and sampling times */
  for (size_t index = 0; index < ARRAY_SIZE(smpr); ++index)
    reg->SMPR[index] = smpr[ARRAY_SIZE(smpr) - index - 1];
  for (size_t index = 0; index < ARRAY_SIZE(sqr); ++index)
    reg->SQR[index] = sqr[ARRAY_SIZE(sqr) - index - 1];

  /* Configure the peripheral and start conversions */
  reg->CR1 = interface->control1;
  reg->CR2 = interface->control2;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct AdcDma *interface)
{
  STM_ADC_Type * const reg = interface->base.reg;

  reg->CR2 = 0;
  reg->CR1 = 0;
  dmaDisable(interface->dma);
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
  assert(interface->count > 0 && interface->count <= 16);

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

  interface->control1 = CR1_SCAN;

  if (config->accuracy)
    interface->control1 |= CR1_RES((12 - config->accuracy) >> 1);
  else
    interface->control1 |= CR1_RES(RES_12BIT);

  interface->control2 = CR2_ADON | CR2_DMA | CR2_DDS | CR2_ALIGN
      | (CR2_EXTSEL(config->event) & CR2_EXTSEL_MASK)
      | CR2_EXTTRIG | CR2_EXTEN(adcEncodeSensitivity(config->sensitivity));

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
  interface->control2 |= CR2_CAL;
#endif

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
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
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
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct AdcDma * const interface = object;

#ifdef CONFIG_PLATFORM_STM32_ADC_BASIC
  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      startCalibration(interface);
      return E_OK;

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
