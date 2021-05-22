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
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop automatic conversion */
  reg->CR = 0;
  /* Disable further DMA requests */
  reg->INTEN = 0;

  // TODO Check DMA errors
//  dmaDisable(interface->dma); // XXX

  for (size_t index = 0; index < interface->count; ++index)
    interface->output[index] = interface->buffer[index];

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void resetDmaBuffers(struct AdcDma *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  dmaClear(interface->dma);

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
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      }
  };
  const struct GpDmaCircularConfig dmaConfig = {
      .number = number,
      .event = GPDMA_ADC0 + config->channel,
      .type = GPDMA_TYPE_P2M,
      .channel = config->dma,
      .oneshot = true,
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

  interface->mask = 0;

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
    uint32_t mask = INTEN_AD(channel);

    assert(!(interface->mask & ~(mask - 1)));

    interface->base.control |= CR_SEL(1 << channel);
    interface->mask |= mask;

    ++index;
  }

  return index;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;

  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    interface->callback = 0;
    interface->blocking = true;

    /* Initialize input pins */
    interface->count = setupPins(interface, config->pins);

    if (config->event == ADC_BURST)
      interface->base.control |= CR_BURST;
    else
      interface->base.control |= CR_START(config->event);

    if (!setupDmaChannel(interface, config, interface->count))
      return E_ERROR;
    resetDmaBuffers(interface);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
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
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, 0, object) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      if (adcSetInstance(interface->base.channel, object, 0))
        irqDisable(interface->base.irq);
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
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Ensure proper alignment of the output buffer */
  assert((uintptr_t)buffer % 2 == 0);
  /* Ensure the buffer has enough space and is aligned to a size of a result */
  assert(length && (length % (interface->count * sizeof(uint16_t)) == 0));

  interface->output = buffer;

  /* Clear pending requests */
  for (size_t index = 0; index < interface->count; ++index)
    (void)reg->DR[interface->pins[index].channel];

  if (dmaEnable(interface->dma) != E_OK)
    return 0;

  /* Enable DMA requests */
  reg->INTEN = interface->mask;
  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;

  if (interface->blocking)
    while (dmaStatus(interface->dma) == E_BUSY);

  return interface->count * sizeof(uint16_t);
}
