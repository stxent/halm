/*
 * dac.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <platform/nxp/dac_dma.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/gen_1/dac_defs.h>
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
  LPC_DAC_Type * const reg = interface->parent.reg;
  const uint32_t index = dmaIndex(interface->dma);

  if (index & 1)
  {
    /* Page is half-empty event */
    if (!interface->updated)
      dmaStop(interface->dma);
    else
      interface->updated = false;
  }
  else
  {
    /* Page changed event */
    if (dmaStatus(interface->dma) == E_OK)
    {
      if (interface->updated)
      {
        dmaStart(interface->dma, (void *)&reg->CR, interface->buffer,
            interface->size * 4);
      }
      else
        reg->CTRL &= ~(CTRL_INT_DMA_REQ | CTRL_CNT_ENA);
    }

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct DacDma *interface,
    const struct DacDmaConfig *config)
{
  const uint32_t elements = config->size >> 1;
  const struct GpDmaListConfig channelConfig = {
      .event = GPDMA_DAC,
      .channel = config->channel,
      .source.increment = true,
      .destination.increment = false,
      .type = GPDMA_TYPE_M2P,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .number = 4,
      .size = elements,
      .circular = true,
      .silent = false
  };

  interface->dma = init(GpDmaList, &channelConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaCallback(interface->dma, dmaHandler, interface);

  interface->buffer = malloc(elements * 4 * sizeof(interface->buffer[0]));
  if (!interface->buffer)
    return E_MEMORY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacDmaConfig * const config = configBase;
  const struct DacBaseConfig parentConfig = {
      .pin = config->pin
  };
  struct DacDma * const interface = object;
  enum result res;

  if (!config->frequency || !config->size)
    return E_VALUE;

  /* Call base class constructor */
  if ((res = DacBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;
  interface->size = config->size;
  interface->updated = false;

  LPC_DAC_Type * const reg = interface->parent.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  reg->CNTVAL = dacGetClock(object) / config->frequency - 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  struct DacDma * const interface = object;

  free(interface->buffer);
  deinit(interface->dma);

  DacBase->deinit(object);
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

    case IF_WIDTH:
      *((uint32_t *)data) = DAC_RESOLUTION;
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
  LPC_DAC_Type * const reg = interface->parent.reg;

  /* Outgoing buffer length should be equal to internal buffer length */
  assert(length == interface->size * sizeof(interface->buffer[0]));

  if (!length)
    return 0;

  if (dmaStatus(interface->dma) == E_OK) {
    /*
     * Previous transfer will be continued. When index is odd number the
     * transfer will be restarted due to buffer underflow.
     */
    uint32_t index = dmaIndex(interface->dma);

    index = 1 - (index >> 1);
    memcpy(interface->buffer + index * interface->size, buffer, length);
    interface->updated = true;
  } else {
    /* Start new transfer */
    memcpy(interface->buffer, buffer, length);
    interface->updated = false;

    const enum result res = dmaStart(interface->dma, (void *)&reg->CR,
        interface->buffer, interface->size * 4);

    if (res != E_OK)
      return 0;

    /* Enable counter to generate memory access requests */
    reg->CTRL |= CTRL_CNT_ENA;
  }

  return length;
}
