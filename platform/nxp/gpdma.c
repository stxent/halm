/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/platform_defs.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_defs.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum result);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelCallback(void *, void (*)(void *), void *);
static enum result channelStart(void *, void *, const void *, uint32_t);
static enum result channelStatus(const void *);
static void channelStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDma),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
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

  channel->error = res != E_OK;

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpDmaConfig * const config = configPtr;
  const struct GpDmaBaseConfig parentConfig = {
      .channel = config->channel,
      .event = config->event,
      .type = config->type
  };
  struct GpDma * const channel = object;
  enum result res;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &parentConfig)) != E_OK)
    return res;

  channel->parent.handler = interruptHandler;
  channel->callback = 0;
  channel->error = false;

  channel->parent.control |= CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->parent.config |= CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;

  /* Set four-byte burst size by default */
  uint8_t dstBurst = DMA_BURST_4, srcBurst = DMA_BURST_4;

  switch (config->type)
  {
    case GPDMA_TYPE_M2P:
      dstBurst = config->burst;
      break;

    case GPDMA_TYPE_P2M:
      srcBurst = config->burst;
      break;

    default:
      break;
  }
  /* Two-byte burst requests are unsupported */
  if (srcBurst >= DMA_BURST_4)
    --srcBurst;
  if (dstBurst >= DMA_BURST_4)
    --dstBurst;
  channel->parent.control |= CONTROL_SRC_BURST(srcBurst)
      | CONTROL_DST_BURST(dstBurst);

  if (config->source.increment)
    channel->parent.control |= CONTROL_SRC_INC;
  if (config->destination.increment)
    channel->parent.control |= CONTROL_DST_INC;

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
static enum result channelStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDma * const channel = object;

  if (size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

  if (gpDmaSetDescriptor(channel->parent.number, object) != E_OK)
    return E_BUSY;

  gpDmaSetupMux(object);
  channel->error = false;

  const uint32_t request = 1 << channel->parent.number;
  LPC_GPDMACH_Type * const reg = channel->parent.reg;

  reg->SRCADDR = (uint32_t)source;
  reg->DESTADDR = (uint32_t)destination;
  reg->CONTROL = channel->parent.control | CONTROL_SIZE(size);
  reg->CONFIG = channel->parent.config;
  reg->LLI = 0;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR |= request;
  LPC_GPDMA->INTERRCLEAR |= request;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDma * const channel = object;
  const LPC_GPDMACH_Type * const reg = channel->parent.reg;

  if (channel->error)
    return E_ERROR;

  return gpDmaGetDescriptor(channel->parent.number) == object
      && (reg->CONFIG & CONFIG_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  LPC_GPDMACH_Type * const reg = ((struct GpDma *)object)->parent.reg;

  reg->CONFIG &= ~CONFIG_ENABLE;
}
