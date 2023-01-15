/*
 * adc_dma.c
 * Copyright (C) 2015, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_dma.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/gpdma_circular.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static void resetDmaBuffers(struct AdcDma *);
static bool setupDmaChannel(struct AdcDma *, const struct AdcDmaConfig *,
    size_t);
static size_t setupPins(struct AdcDma *, const PinNumber *);
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
#define adcDeinit deletedDestructorTrap
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
    .write = 0
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void resetDmaBuffers(struct AdcDma *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  for (size_t index = 0; index < interface->count; ++index)
  {
    dmaAppend(interface->dma, &interface->buffer[index],
        (const void *)&reg->DR[interface->pins[index].channel], 1);
  }
}
/*----------------------------------------------------------------------------*/
static bool setupDmaChannel(struct AdcDma *interface,
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

  if (interface->dma)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static size_t setupPins(struct AdcDma *interface, const PinNumber *pins)
{
  size_t index = 0;
  uint32_t enabled = 0;

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
    assert(!(enabled >> channel));

    enabled |= 1 << channel;
    ++index;
  }

  assert(enabled != 0);
  interface->base.control |= CR_SEL(enabled);
  interface->mask = enabled;

  return index;
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct AdcDma *interface)
{
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
  /* Disable interrupts */
  reg->INTEN = 0;

  dmaDisable(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config);
  assert(config->pins);
  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;
  enum Result res;

  /* Call base class constructor */
  res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    interface->callback = 0;
    memset(interface->buffer, 0, sizeof(interface->buffer));

    if (config->event == ADC_BURST)
      interface->base.control |= CR_BURST;
    else
      interface->base.control |= CR_START(config->event);

    /* Initialize input pins */
    interface->count = setupPins(interface, config->pins);

    if (setupDmaChannel(interface, config, interface->count))
    {
      if (!config->shared)
        res = startConversion(interface) ? E_OK : E_ERROR;
    }
    else
      res = E_ERROR;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

  stopConversion(interface);
  deinit(interface->dma);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
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
  struct AdcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
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
  struct AdcDma * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  memcpy(buffer, interface->buffer, chunk);
  return chunk;
}
