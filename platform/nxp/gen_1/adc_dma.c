/*
 * adc_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/adc_dma.h>
#include <platform/nxp/gen_1/adc_defs.h>
#include <platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_COUNT 2
#define SAMPLE_SIZE sizeof(uint16_t)
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
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;
  const uint32_t count = dmaCount(interface->dma);
  const enum result res = dmaStatus(interface->dma);

  /* Scatter-gather transfer finished */
  if (res != E_BUSY)
  {
    /* Stop automatic conversion */
    reg->CR &= ~CR_START_MASK;
    /* Disable requests */
    reg->INTEN = 0;

    adcUnitUnregister(unit);
  }

  /*
   * Each block consists of two buffers. Call user function
   * at the end of block or at the end of transfer.
   */
  if ((res != E_BUSY || !(count & 1)) && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config)
{
  const struct GpDmaListConfig dmaConfig = {
      .event = GPDMA_ADC0 + config->parent->base.channel,
      .channel = config->dma,
      .source.increment = false,
      .destination.increment = true,
      .type = GPDMA_TYPE_P2M,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .number = BLOCK_COUNT << 1,
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

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);

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

  adcReleasePin(interface->pin);
  deinit(interface->dma);
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
    case IF_RX_CAPACITY:
      *(uint32_t *)data = BLOCK_COUNT - ((dmaCount(interface->dma) + 1) >> 1);
      return E_OK;

    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_WIDTH:
      *(uint32_t *)data = ADC_RESOLUTION;
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
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;
  const uint32_t samples = length / SAMPLE_SIZE;

  if (!samples)
    return 0;

  /* Strict requirements on the buffer length */
  assert(samples > (interface->size >> 1) && samples <= interface->size);

  const bool ongoing = dmaStatus(interface->dma) == E_BUSY;

  if (!ongoing)
  {
    if (adcUnitRegister(unit, 0, interface) != E_OK)
      return 0;
  }

  /* When previous transfer is ongoing it will be continued */
  const enum result res = dmaStart(interface->dma, buffer,
      (void *)&reg->DR[interface->pin.channel], samples);

  if (res != E_OK)
  {
    adcUnitUnregister(unit);
    return 0;
  }

  /* Enable DMA requests */
  reg->INTEN = INTEN_AD(interface->pin.channel);

  if (!ongoing)
  {
    /* Set conversion channel */
    reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL(interface->pin.channel);
    /* Start the conversion */
    reg->CR |= CR_START(interface->event);
  }

  return samples * SAMPLE_SIZE;
}
