/*
 * dac.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <platform/nxp/dac_dma.h>
#include <platform/nxp/dac_defs.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
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
const struct InterfaceClass *DacDma = &dacTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct DacDma *interface = object;
  LPC_DAC_Type *reg = interface->parent.reg;
  uint32_t index = dmaIndex(interface->dma);

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
    if (!dmaActive(interface->dma))
    {
      if (interface->updated)
        dmaListExecute(interface->dma);
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
  const struct GpDmaListConfig channelConfig = {
      .event = GPDMA_DAC,
      .channel = config->channel,
      .source.increment = true,
      .destination.increment = false,
      .type = GPDMA_TYPE_M2P,
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .size = 4,
      .circular = true,
      .silence = false
  };
  LPC_DAC_Type *reg = interface->parent.reg;

  interface->dma = init(GpDmaList, &channelConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaCallback(interface->dma, dmaHandler, interface);

  const uint32_t chunkLength = config->length >> 1;

  interface->buffer = malloc(chunkLength * 4 * sizeof(uint16_t));
  if (!interface->buffer)
    return E_MEMORY;

  for (uint8_t index = 0; index < 4; ++index)
  {
    dmaListAppend(interface->dma, (void *)&reg->CR, interface->buffer
        + index * chunkLength, chunkLength);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configPtr)
{
  const struct DacDmaConfig * const config = configPtr;
  const struct DacBaseConfig parentConfig = {
      .pin = config->pin
  };
  struct DacDma *interface = object;
  enum result res;

  if (!config->frequency || !config->length)
    return E_VALUE;

  /* Call base class constructor */
  if ((res = DacBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;
  interface->length = config->length;
  interface->updated = false;

  LPC_DAC_Type *reg = interface->parent.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  reg->CNTVAL = dacGetClock(object) / config->frequency - 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  struct DacDma *interface = object;

  free(interface->buffer);
  deinit(interface->dma);

  DacBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result dacCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DacDma *interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct DacDma *interface = object;

  switch (option)
  {
    case IF_STATUS:
      return dmaActive(interface->dma) ? E_BUSY : E_OK;

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
  struct DacDma *interface = object;
  LPC_DAC_Type *reg = interface->parent.reg;

  /* Outgoing buffer length should be equal to internal buffer length */
  assert(length == interface->length * sizeof(uint16_t));

  if (!length)
    return 0;

  if (dmaActive(interface->dma)) {
    /*
     * Previous transfer will be continued. When index is odd number the
     * transfer will be restarted due to buffer underflow.
     */
    uint32_t index = dmaIndex(interface->dma);

    index = 1 - (index >> 1);
    memcpy(interface->buffer + index * interface->length, buffer, length);
    //TODO Check whether flag can be set without transfer restarting
    interface->updated = true;
  } else {
    /* Start new transfer */
    memcpy(interface->buffer, buffer, length);
    interface->updated = false;

    dmaListExecute(interface->dma);

    /* Enable counter to generate memory access requests */
    reg->CTRL |= CTRL_CNT_ENA;
  }

  return length;
}
