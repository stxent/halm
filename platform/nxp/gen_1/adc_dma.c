/*
 * adc_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/adc_dma.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static enum result dmaSetup(struct AdcDma *, const struct AdcDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcCallback(void *, void (*)(void *), void *);
static enum result adcGet(void *, enum ifOption, void *);
static enum result adcSet(void *, enum ifOption, const void *);
static uint32_t adcRead(void *, uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct AdcDma),
    .init = adcInit,
    .deinit = adcDeinit,

    .callback = adcCallback,
    .get = adcGet,
    .set = adcSet,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcDma = &adcTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;
  LPC_ADC_Type * const reg = interface->unit->parent.reg;
  const uint32_t index = dmaIndex(interface->dma);

  if (!(index & 0x01))
  {
    /* Scatter-gather transfer finished */
    if (dmaStatus(interface->dma) != E_BUSY)
    {
      /* Stop automatic conversion */
      reg->CR &= ~CR_START_MASK;
      /* Unregister interface when all conversions are done */
      adcUnitUnregister(interface->unit);
    }

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config)
{
  const struct GpDmaListConfig dmaConfig = {
      .event = GPDMA_ADC,
      .channel = config->channel,
      .source.increment = false,
      .destination.increment = true,
      .type = GPDMA_TYPE_P2M,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .number = 4,
      .size = config->size >> 1,
      .silent = false
  };

  interface->dma = init(GpDmaList, &dmaConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaCallback(interface->dma, dmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  struct AdcDma * const interface = object;
  enum result res;

  assert(config->event < ADC_EVENT_END);

  res = adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);
  if (res != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;
  interface->event = config->event + 1;
  interface->size = config->size;
  interface->unit = config->parent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  const struct AdcDma * const interface = object;

  deinit(interface->dma);
  adcReleasePin(interface->pin);
}
/*----------------------------------------------------------------------------*/
static enum result adcCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcGet(void *object, enum ifOption option, void *data)
{
  struct AdcDma * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_WIDTH:
      *((uint32_t *)data) = ADC_RESOLUTION;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t adcRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct AdcDma * const interface = object;
  LPC_ADC_Type * const reg = interface->unit->parent.reg;
  const uint32_t samples = length / sizeof(uint16_t);

  if (!samples)
    return 0;

  /* Strict requirements on the buffer length */
  assert(samples > (interface->size >> 1) && samples <= interface->size);

  if (adcUnitRegister(interface->unit, 0, interface) != E_OK)
    return 0;

  const bool ongoing = dmaStatus(interface->dma) == E_BUSY;

  /* When previous transfer is ongoing it will be continued */
  const enum result res = dmaStart(interface->dma, buffer,
      (void *)&reg->DR[interface->pin.channel], samples);

  if (res != E_OK)
    return 0;

  if (!ongoing)
  {
    /* Set conversion channel */
    reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL(interface->pin.channel);
    /* Start the conversion */
    reg->CR |= CR_START(interface->event);
  }

  return samples * sizeof(uint16_t);
}
