/*
 * gpdma_list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <halm/irq.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/nxp/gpdma_list.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static void appendItem(void *, void *, const void *, unsigned int);
static void interruptHandler(void *, enum result);
static void startTransfer(struct GpDmaList *, const struct GpDmaListEntry *);
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
    .size = sizeof(struct GpDmaList),
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
const struct DmaClass * const GpDmaList = &channelTable;
/*----------------------------------------------------------------------------*/
static void appendItem(void *object, void *destination, const void *source,
    unsigned int size)
{
  struct GpDmaList * const channel = object;
  struct GpDmaListEntry * const entry = channel->list + channel->current;
  struct GpDmaListEntry *previous = 0;

  if (channel->queued)
  {
    /* There are initialized entries in the list */
    previous = channel->current ? entry : channel->list + channel->capacity;
    previous = previous - 1;
  }

  if (++channel->current >= channel->capacity)
    channel->current = 0;

  entry->source = (uint32_t)source;
  entry->destination = (uint32_t)destination;
  entry->control = channel->base.control | CONTROL_SIZE(size);
  entry->next = 0;

  if (previous)
  {
    /* Link current element to the previous one */
    previous->next = (uint32_t)entry;
  }

  ++channel->queued;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDmaList * const channel = object;

  channel->queued = channel->silent ? 0 : channel->queued - 1;
  channel->error = res != E_OK && res != E_BUSY;

  if (res == E_OK)
  {
    if (channel->queued)
    {
      /* Underrun occurred */
      unsigned int index = channel->current;

      /* Calculate index of the first stalled chunk */
      if (index >= channel->queued)
        index -= channel->queued;
      else
        index += channel->capacity - channel->queued;

      /* Restart stalled transfer */
      startTransfer(channel, channel->list + index);
    }
    else
      gpDmaClearDescriptor(channel->base.number);
  }

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct GpDmaList *channel,
    const struct GpDmaListEntry *entry)
{
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  reg->SRCADDR = entry->source;
  reg->DESTADDR = entry->destination;
  reg->CONTROL = entry->control;
  reg->LLI = entry->next;
  reg->CONFIG = channel->base.config;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaListConfig * const config = configBase;
  const struct GpDmaBaseConfig baseConfig = {
      .channel = config->channel,
      .event = config->event,
      .type = config->type
  };
  struct GpDmaList * const channel = object;
  enum result res;

  assert(config->burst != DMA_BURST_2 && config->burst <= DMA_BURST_256);
  assert(config->width <= DMA_WIDTH_WORD);
  assert(config->number);
  assert(config->size && config->size <= GPDMA_MAX_TRANSFER);

  /* Allocator must provide memory chunks aligned along 4-byte boundary */
  channel->list = malloc(sizeof(struct GpDmaListEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &baseConfig)) != E_OK)
    return res;

  channel->base.control |= CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->base.config |= CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->capacity = config->number;
  channel->silent = config->silent;
  channel->size = config->size;
  channel->width = 1 << config->width;

  channel->current = 0;
  channel->queued = 0;
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
  struct GpDmaList * const channel = object;

  free(channel->list);
  GpDmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDmaList * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static size_t channelCount(const void *object)
{
  const struct GpDmaList * const channel = object;

  return channel->queued;
}
/*----------------------------------------------------------------------------*/
static enum result channelReconfigure(void *object, const void *configBase)
{
  const struct GpDmaListRuntimeConfig * const config = configBase;
  struct GpDmaList * const channel = object;
  uint32_t control;

  control = channel->base.control & ~(CONTROL_SRC_INC | CONTROL_DST_INC);

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
  struct GpDmaList * const channel = object;
  uint8_t *destinationBuffer = destination;
  const uint8_t *sourceBuffer = source;

  assert(size);

  if (size > (size_t)((channel->capacity - channel->queued) * channel->size))
    return E_VALUE;

  const irqState state = irqSave();
  const bool active = gpDmaGetDescriptor(channel->base.number) == object;

  if (active && channel->silent)
  {
    irqRestore(state);
    return E_BUSY;
  }

  if (!active)
  {
    if (gpDmaSetDescriptor(channel->base.number, object) != E_OK)
    {
      irqRestore(state);
      return E_BUSY;
    }

    gpDmaSetMux(object);

    channel->current = 0;
    channel->queued = 0;
    channel->error = false;
  }

  for (size_t offset = 0; offset < size;)
  {
    const unsigned int chunkLength = (size - offset >= channel->size) ?
        channel->size : (size - offset);
    const unsigned int totalLength = chunkLength * channel->width;

    appendItem(channel, destinationBuffer, sourceBuffer, chunkLength);

    offset += chunkLength;
    if (channel->base.control & CONTROL_DST_INC)
      destinationBuffer += totalLength;
    if (channel->base.control & CONTROL_SRC_INC)
      sourceBuffer += totalLength;
  }

  irqRestore(state);

  if (!active)
  {
    const uint32_t request = 1 << channel->base.number;

    LPC_GPDMA->INTTCCLEAR = request;
    LPC_GPDMA->INTERRCLEAR = request;

    startTransfer(channel, channel->list);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDmaList * const channel = object;

  if (channel->error)
    return E_ERROR;

  if (gpDmaGetDescriptor(channel->base.number) == object)
    return E_BUSY;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  struct GpDmaList * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
  irqState state;

  state = irqSave();
  if (channel->queued > 1)
    channel->queued = 1;
  irqRestore(state);

  /* Stop immediately */
  reg->CONFIG &= ~CONFIG_ENABLE;
}
