/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gpdma.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/platform_defs.h>
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static bool channelActive(void *);
static void channelCallback(void *, void (*)(void *), void *);
static uint32_t channelIndex(void *);
static enum result channelStart(void *, void *, const void *, uint32_t);
static void channelStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDma),
    .init = channelInit,
    .deinit = channelDeinit,

    .active = channelActive,
    .callback = channelCallback,
    .index = channelIndex,
    .start = channelStart,
    .stop = channelStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass *GpDma = &channelTable;
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configPtr)
{
  const struct GpDmaConfig * const config = configPtr;
  const struct GpDmaBaseConfig parentConfig = {
      .channel = config->channel,
      .event = config->event,
      .type = config->type
  };
  struct GpDma *channel = object;
  enum result res;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &parentConfig)) != E_OK)
    return res;

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
  struct GpDma *channel = object;

  GpDmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static bool channelActive(void *object)
{
  struct GpDma *channel = object;
  LPC_GPDMACH_Type *reg = channel->parent.reg;

  return gpDmaGetDescriptor(channel->parent.number) == object
      && (reg->CONFIG & CONFIG_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDma *channel = object;

  channel->parent.callback = callback;
  channel->parent.callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static uint32_t channelIndex(void *object)
{
  LPC_GPDMACH_Type *reg = ((struct GpDma *)object)->parent.reg;

  //FIXME Rewrite
  return reg->CONTROL & CONTROL_SIZE_MASK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDma *channel = object;

  if (size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

  if (gpDmaSetDescriptor(channel->parent.number, object) != E_OK)
    return E_BUSY;

  LPC_SC->DMAREQSEL = (LPC_SC->DMAREQSEL & channel->parent.muxMask)
      | channel->parent.muxValue;

  const uint32_t request = 1 << channel->parent.number;
  LPC_GPDMACH_Type *reg = channel->parent.reg;

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
static void channelStop(void *object)
{
  LPC_GPDMACH_Type *reg = ((struct GpDma *)object)->parent.reg;

  reg->CONFIG &= ~CONFIG_ENABLE;
}
