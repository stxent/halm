/*
 * adc_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/adc_dma.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <halm/platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_COUNT 2
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct AdcDma *, const struct AdcDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, enum IfParameter, void *);
static enum Result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
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
  bool event = false;

  /* Scatter-gather transfer finished */
  if (dmaStatus(interface->dma) != E_BUSY)
  {
    LPC_ADC_Type * const reg = interface->base.reg;

    /* Stop automatic conversion */
    reg->CR = 0;
    /* Disable further requests */
    reg->INTEN = 0;

    adcResetInstance(interface->base.channel);
    event = true;
  }
  else if ((dmaPending(interface->dma) & 1) == 0)
  {
    /*
     * Each block consists of two buffers. Call user function
     * at the end of the block or at the end of the transfer.
     */
    event = true;
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config)
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
          .increment = true
      }
  };
  const struct GpDmaListConfig dmaConfig = {
      .number = BUFFER_COUNT << 1,
      .event = GPDMA_ADC0 + config->channel,
      .type = GPDMA_TYPE_P2M,
      .channel = config->dma,
      .silent = false
  };

  interface->dma = init(GpDmaList, &dmaConfig);

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
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel
  };
  struct AdcDma * const interface = object;

  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    if (!dmaSetup(interface, config))
      return E_ERROR;

    /* Enable analog function on the input pin */
    interface->pin = adcConfigPin(&interface->base, config->pin);

    interface->callback = 0;
    interface->base.control |= CR_SEL_CHANNEL(interface->pin.channel);

    if (config->event == ADC_BURST)
      interface->base.control |= CR_BURST;
    else
      interface->base.control |= CR_START(config->event);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

  adcReleasePin(interface->pin);
  deinit(interface->dma);

  if (AdcBase->deinit)
    AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct AdcDma * const interface = object;

  switch (parameter)
  {
    case IF_AVAILABLE:
      *(size_t *)data = BUFFER_COUNT - ((dmaPending(interface->dma) + 1) >> 1);
      return E_OK;

    case IF_STATUS:
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  /* Ensure proper alignment of the output buffer */
  assert(!((uintptr_t)buffer & 1));

  /* The buffer should be at least 2 samples long */
  if (length < 2 * sizeof(uint16_t))
    return 0;

  struct AdcDma * const interface = object;
  LPC_ADC_Type * const reg = interface->base.reg;
  const size_t samples = length / sizeof(uint16_t);
  const uint8_t channel = interface->pin.channel;

  /* Prepare linked list of DMA descriptors */
  const size_t parts[] = {samples / 2, samples - samples / 2};
  const uint16_t * const source = (const uint16_t *)&reg->DR[channel];
  uint16_t * const sink = buffer;

  /* When the previous transfer is ongoing it will be continued */
  dmaAppend(interface->dma, sink, source, parts[0]);
  dmaAppend(interface->dma, sink + parts[0], source, parts[1]);

  if (dmaStatus(interface->dma) != E_BUSY)
  {
    /* Clear pending requests */
    (void)(*source);

    if (!adcSetInstance(interface->base.channel, &interface->base))
      goto error;

    if (dmaEnable(interface->dma) != E_OK)
    {
      adcResetInstance(interface->base.channel);
      goto error;
    }

    /* Enable DMA requests */
    reg->INTEN = INTEN_AD(channel);
    /* Reconfigure peripheral and start the conversion */
    reg->CR = interface->base.control;
  }

  return samples * sizeof(uint16_t);

error:
  dmaClear(interface->dma);
  return 0;
}
