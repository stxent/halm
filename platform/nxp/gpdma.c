/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gpdma.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum result);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelCallback(void *, void (*)(void *), void *);
static size_t channelCount(const void *);
static enum result channelReconfigure(void *, const void *);
static enum result channelStart(void *, void *, const void *, size_t);
static enum result channelStatus(const void *);
static void channelStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDma),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
    .count = channelCount,
    .reconfigure = channelReconfigure,
    .start = channelStart,
    .status = channelStatus,
    .stop = channelStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass * const GpDma = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDma * const channel = object;

  gpDmaClearDescriptor(channel->base.number);
  channel->error = res != E_OK;

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaConfig * const config = configBase;
  const struct GpDmaBaseConfig baseConfig = {
      .channel = config->channel,
      .event = config->event,
      .type = config->type
  };
  struct GpDma * const channel = object;
  enum result res;

  assert(config->burst != DMA_BURST_2 && config->burst <= DMA_BURST_256);
  assert(config->width <= DMA_WIDTH_WORD);

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &baseConfig)) != E_OK)
    return res;

  channel->base.control |= CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->base.config |= CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->error = false;

  /* Set four-byte burst size by default */
  uint8_t dstBurst = DMA_BURST_4, srcBurst = DMA_BURST_4;

  if (config->type == GPDMA_TYPE_M2P)
    dstBurst = config->burst;
  if (config->type == GPDMA_TYPE_P2M)
    srcBurst = config->burst;

  /* Two-byte burst requests are unsupported */
  if (srcBurst >= DMA_BURST_4)
    --srcBurst;
  if (dstBurst >= DMA_BURST_4)
    --dstBurst;

  channel->base.control |= CONTROL_SRC_BURST(srcBurst)
      | CONTROL_DST_BURST(dstBurst);

  if (config->source.increment)
    channel->base.control |= CONTROL_SRC_INC;
  if (config->destination.increment)
    channel->base.control |= CONTROL_DST_INC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  GpDmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDma * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static size_t channelCount(const void *object)
{
  const struct GpDma * const channel = object;
  const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  if (gpDmaGetDescriptor(channel->base.number) != object)
    return 0;

  return CONTROL_SIZE_VALUE(channel->base.control)
      - CONTROL_SIZE_VALUE(reg->CONTROL);
}
/*----------------------------------------------------------------------------*/
static enum result channelReconfigure(void *object, const void *configBase)
{
  const struct GpDmaRuntimeConfig * const config = configBase;
  struct GpDma * const channel = object;
  uint32_t control = channel->base.control
      & ~(CONTROL_SRC_INC | CONTROL_DST_INC);

  if (config->source.increment)
    control |= CONTROL_SRC_INC;
  if (config->destination.increment)
    control |= CONTROL_DST_INC;

  channel->base.control = control;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStart(void *object, void *destination,
    const void *source, size_t size)
{
  struct GpDma * const channel = object;

  assert(size && size <= GPDMA_MAX_TRANSFER);

  if (gpDmaSetDescriptor(channel->base.number, object) != E_OK)
    return E_BUSY;

  gpDmaSetMux(object);
  channel->base.control = (channel->base.control & ~CONTROL_SIZE_MASK)
      | CONTROL_SIZE(size);
  channel->error = false;

  const uint32_t request = 1 << channel->base.number;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  reg->SRCADDR = (uint32_t)source;
  reg->DESTADDR = (uint32_t)destination;
  reg->CONTROL = channel->base.control;
  reg->CONFIG = channel->base.config;
  reg->LLI = 0;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR = request;
  LPC_GPDMA->INTERRCLEAR = request;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDma * const channel = object;

  if (channel->error)
    return E_ERROR;

  return gpDmaGetDescriptor(channel->base.number) == object ? E_BUSY : E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  LPC_GPDMA_CHANNEL_Type * const reg = ((struct GpDma *)object)->base.reg;

  reg->CONFIG &= ~CONFIG_ENABLE;
}
