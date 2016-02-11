/*
 * dac_dma.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/dac_dma.h>
#include <platform/nxp/gen_1/dac_defs.h>
#include <platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_COUNT 2
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object);
static enum result dmaSetup(struct DacDma *, const struct DacDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
static enum result dacCallback(void *, void (*)(void *), void *);
static enum result dacGet(void *, enum ifOption, void *);
static enum result dacSet(void *, enum ifOption, const void *);
static uint32_t dacWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass dacTable = {
    .size = sizeof(struct DacDma),
    .init = dacInit,
    .deinit = dacDeinit,

    .callback = dacCallback,
    .get = dacGet,
    .set = dacSet,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const DacDma = &dacTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct DacDma * const interface = object;
  LPC_DAC_Type * const reg = interface->base.reg;
  const uint32_t count = dmaCount(interface->dma);
  const enum result res = dmaStatus(interface->dma);

  /* Scatter-gather transfer finished */
  if (res != E_BUSY)
    reg->CTRL &= ~(CTRL_INT_DMA_REQ | CTRL_CNT_ENA);

  if ((res != E_BUSY || !(count & 1)) && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct DacDma *interface,
    const struct DacDmaConfig *config)
{
  const struct GpDmaListConfig channelConfig = {
      .event = GPDMA_DAC,
      .channel = config->channel,
      .source.increment = true,
      .destination.increment = false,
      .type = GPDMA_TYPE_M2P,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .number = BLOCK_COUNT << 1,
      .size = config->size >> 1,
      .silent = false
  };

  interface->dma = init(GpDmaList, &channelConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaCallback(interface->dma, dmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacDmaConfig * const config = configBase;
  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  struct DacDma * const interface = object;
  enum result res;

  assert(config->frequency);
  assert(config->size);

  /* Call base class constructor */
  if ((res = DacBase->init(object, &baseConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;
  interface->size = config->size;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  reg->CNTVAL = dacGetClock(object) / config->frequency - 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  struct DacDma * const interface = object;

  deinit(interface->dma);
  DacBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result dacCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DacDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacGet(void *object, enum ifOption option, void *data)
{
  struct DacDma * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_TX_CAPACITY:
      *(uint32_t *)data = BLOCK_COUNT - ((dmaCount(interface->dma) + 1) >> 1);
      return E_OK;

    case IF_WIDTH:
      *(uint32_t *)data = DAC_RESOLUTION;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result dacSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t dacWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct DacDma * const interface = object;
  LPC_DAC_Type * const reg = interface->base.reg;
  const uint32_t samples = length / SAMPLE_SIZE;

  if (!samples)
    return 0;

  /* Strict requirements on the buffer length */
  assert(samples > (interface->size >> 1) && samples <= interface->size);

  const bool ongoing = dmaStatus(interface->dma) == E_BUSY;

  /* When the transfer is already active it will be continued */
  const enum result res = dmaStart(interface->dma, (void *)&reg->CR,
      buffer, samples);

  if (res != E_OK)
    return 0;

  if (!ongoing)
  {
    /* Enable counter to generate memory access requests */
    reg->CTRL |= CTRL_CNT_ENA;
  }

  return samples * SAMPLE_SIZE;
}
