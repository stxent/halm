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
#define BLOCK_COUNT 2
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static enum Result dmaSetup(struct AdcDma *, const struct AdcDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum Result adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, enum IfParameter, void *);
static enum Result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
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
const struct InterfaceClass * const AdcDma = &adcTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  bool event = false;

  /* Scatter-gather transfer finished */
  if (dmaStatus(interface->dma) != E_BUSY)
  {
    LPC_ADC_Type * const reg = unit->base.reg;

    /* Stop automatic conversion */
    reg->CR &= ~(CR_BURST | CR_START_MASK);
    /* Disable further requests */
    reg->INTEN = 0;

    adcUnitUnregister(unit);
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
static enum Result dmaSetup(struct AdcDma *interface,
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
      .number = BLOCK_COUNT << 1,
      .event = GPDMA_ADC0 + config->parent->base.channel,
      .type = GPDMA_TYPE_P2M,
      .channel = config->dma,
      .silent = false
  };

  interface->dma = init(GpDmaList, &dmaConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaConfigure(interface->dma, &dmaSettings);
  dmaSetCallback(interface->dma, dmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config);

  struct AdcDma * const interface = object;
  enum Result res;

  assert(config->event < ADC_EVENT_END);
  assert(config->event != ADC_SOFTWARE);

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);

  interface->callback = 0;
  interface->event = config->event;
  interface->unit = config->parent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  const struct AdcDma * const interface = object;

  adcReleasePin(interface->pin);
  deinit(interface->dma);
}
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
      *(size_t *)data = BLOCK_COUNT - ((dmaPending(interface->dma) + 1) >> 1);
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
  struct AdcDma * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  const size_t samples = length / sizeof(uint16_t);
  const uint8_t channel = interface->pin.channel;

  /* At least 2 samples */
  assert(samples >= 2);

  const size_t parts[] = {samples / 2, samples - samples / 2};
  LPC_ADC_Type * const reg = unit->base.reg;
  const uint32_t * const source = (const uint32_t *)(reg->DR + channel);
  uint16_t *destination = buffer;

  /* When the previous transfer is ongoing it will be continued */
  dmaAppend(interface->dma, destination, source, parts[0]);
  destination += parts[0];
  dmaAppend(interface->dma, destination, source, parts[1]);

  if (dmaStatus(interface->dma) != E_BUSY)
  {
    /* Clear pending requests */
    (void)(*source);

    if (adcUnitRegister(unit, 0, interface) != E_OK)
      goto error;

    if (dmaEnable(interface->dma) != E_OK)
    {
      adcUnitUnregister(unit);
      goto error;
    }

    /* Enable DMA requests */
    reg->INTEN = INTEN_AD(channel);

    /* Set conversion channel */
    reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL_CHANNEL(channel);

    /* Start the conversion */
    if (interface->event == ADC_BURST)
      reg->CR |= CR_BURST;
    else
      reg->CR |= CR_START(interface->event);
  }

  return samples * sizeof(uint16_t);

error:
  dmaClear(interface->dma);
  return 0;
}
