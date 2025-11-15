/*
 * adc_dma.c
 * Copyright (C) 2015, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_dma.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/gpdma_circular.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct AdcDma *, const struct AdcDmaConfig *, size_t);
static void resetDmaBuffers(struct AdcDma *);
static bool startConversion(struct AdcDma *);
static void stopConversion(struct AdcDma *);
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
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config, size_t number)
{
  static const struct GpDmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      }
  };
  const struct GpDmaCircularConfig dmaConfig = {
      .number = number,
      .event = GPDMA_ADC0 + config->channel,
      .type = GPDMA_TYPE_P2M,
      .channel = config->dma,
      .oneshot = false,
      .silent = true
  };

  interface->dma = init(GpDmaCircular, &dmaConfig);

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
static void resetDmaBuffers(struct AdcDma *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  for (size_t index = 0; index < interface->count; ++index)
  {
    dmaAppend(interface->dma, &interface->buffer[index],
        (const void *)&reg->DR[interface->pins[index].channel],
        sizeof(*interface->buffer));
  }
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct AdcDma *interface)
{
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  LPC_ADC_Type * const reg = interface->base.reg;

  /* Clear pending requests */
  for (size_t index = 0; index < interface->count; ++index)
    (void)reg->DR[interface->pins[index].channel];
  /* Rebuild DMA descriptor chain */
  resetDmaBuffers(interface);

  if (dmaEnable(interface->dma) != E_OK)
    return false;

  /* Enable DMA requests */
  reg->INTEN = interface->mask;
  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct AdcDma *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->CR = 0;
  /* Disable DMA requests */
  reg->INTEN = 0;

  dmaDisable(interface->dma);
  dmaClear(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);
  assert(config->event < ADC_EVENT_END && config->event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  if (!dmaSetup(interface, config, interface->count))
    return E_ERROR;

  interface->buffer =
      malloc((sizeof(uint16_t) + sizeof(struct AdcPin)) * count);
  if (interface->buffer == NULL)
    return E_MEMORY;
  interface->pins = (struct AdcPin *)(interface->buffer + count);

  interface->callback = NULL;
  interface->count = (uint8_t)count;

  if (config->event == ADC_BURST)
    interface->base.control |= CR_BURST;
  else
    interface->base.control |= CR_START(config->event);

  /* Initialize input pins */
  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->base.control |= CR_SEL(mask);
  interface->mask = mask;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

#  ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  if (adcGetInstance(interface->base.channel) == &interface->base)
#  endif
  {
    stopConversion(interface);
  }

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->buffer);

  deinit(interface->dma);
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
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct AdcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      return startConversion(interface) ? E_OK : E_ERROR;

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
  struct AdcDma * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  memcpy(buffer, interface->buffer, chunk);
  return chunk;
}
