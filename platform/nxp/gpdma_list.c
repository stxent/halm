/*
 * gpdma_list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <platform/platform_defs.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
static void appendItem(void *, void *, const void *, uint32_t);
static void clearEntryList(void *);
static void interruptHandler(void *, enum result);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelCallback(void *, void (*)(void *), void *);
static uint32_t channelCount(const void *);
static enum result channelStart(void *, void *, const void *, uint32_t);
static enum result channelStatus(const void *);
static void channelStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDmaList),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
    .count = channelCount,
    .start = channelStart,
    .status = channelStatus,
    .stop = channelStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass * const GpDmaList = &channelTable;
/*----------------------------------------------------------------------------*/
static void appendItem(void *object, void *destination, const void *source,
    uint32_t size)
{
  struct GpDmaList * const channel = object;
  struct GpDmaListEntry * const entry = channel->list + channel->current;

  if (channel->unused < channel->capacity)
  {
    /* There are initialized entries in the list */
    struct GpDmaListEntry * const previous = (channel->current ?
        entry : channel->list + channel->capacity) - 1;

    /* Link current element to the previous one */
    previous->next = (uint32_t)entry;

    if (channel->silent)
      previous->control &= ~CONTROL_INT;
  }

  if (++channel->current >= channel->capacity)
    channel->current = 0;

  entry->source = (uint32_t)source;
  entry->destination = (uint32_t)destination;
  entry->control = channel->parent.control | CONTROL_SIZE(size);
  entry->next = 0;

  if (channel->unused)
    --channel->unused;
}
/*----------------------------------------------------------------------------*/
static void clearEntryList(void *object)
{
  struct GpDmaList * const channel = object;

  channel->unused = channel->capacity;
  channel->status = E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDmaList * const channel = object;

  channel->unused = (res != E_BUSY || channel->silent) ? channel->capacity
      : channel->unused + 1;
  channel->status = (res == E_OK || res == E_BUSY) ? E_OK : res;

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaListConfig * const config = configBase;
  const struct GpDmaBaseConfig parentConfig = {
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

  /* Allocation should produce memory chunks aligned along 4-byte boundary */
  channel->list = malloc(sizeof(struct GpDmaListEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &parentConfig)) != E_OK)
    return res;

  channel->parent.control |= CONTROL_INT | CONTROL_SRC_WIDTH(config->width)
      | CONTROL_DST_WIDTH(config->width);
  channel->parent.config |= CONFIG_TYPE(config->type) | CONFIG_IE | CONFIG_ITC;
  channel->parent.handler = interruptHandler;

  channel->callback = 0;
  channel->capacity = config->number;
  channel->current = 0;
  channel->status = E_OK;
  channel->silent = config->silent;
  channel->size = config->size;
  channel->unused = config->number; /* Initial value is equal to capacity */
  channel->width = 1 << config->width;

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
static uint32_t channelCount(const void *object)
{
  const struct GpDmaList * const channel = object;

  return (uint32_t)(channel->capacity - channel->unused);
}
/*----------------------------------------------------------------------------*/
static enum result channelStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDmaList * const channel = object;
  LPC_GPDMACH_Type * const reg = channel->parent.reg;
  const bool active = gpDmaGetDescriptor(channel->parent.number) == object;

  assert(size);

  if (size > (uint32_t)(channel->unused * channel->size))
    return E_VALUE;

  if (!active)
  {
    if (gpDmaSetDescriptor(channel->parent.number, object) != E_OK)
      return E_BUSY;

    gpDmaSetMux(object);
    clearEntryList(channel);
  }

  uint32_t offset = 0;

  while (offset < size)
  {
    const uint32_t chunk = size - offset >= channel->size ?
        channel->size : size - offset;

    appendItem(channel, destination, source, chunk);

    offset += chunk;
    if (channel->parent.control & CONTROL_DST_INC)
      destination = (void *)((uint32_t)destination + chunk * channel->width);
    if (channel->parent.control & CONTROL_SRC_INC)
      source = (const void *)((uint32_t)source + chunk * channel->width);
  }

  if (!active)
  {
    const struct GpDmaListEntry * const first = channel->list;
    const uint32_t request = 1 << channel->parent.number;

    reg->SRCADDR = first->source;
    reg->DESTADDR = first->destination;
    reg->CONTROL = first->control;
    reg->LLI = first->next;
    reg->CONFIG = channel->parent.config;

    LPC_GPDMA->INTTCCLEAR |= request;
    LPC_GPDMA->INTERRCLEAR |= request;

    /* Start the transfer */
    reg->CONFIG |= CONFIG_ENABLE;
  }
  else
  {
    /* Check whether the channel is still active */
    if (!reg->LLI)
    {
      /* Buffer underflow occurred, transfer should be restarted */
      return E_TIMEOUT;
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDmaList * const channel = object;

  if (channel->status != E_OK)
    return channel->status;
  else if (gpDmaGetDescriptor(channel->parent.number) != object)
    return E_OK;
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  struct GpDmaList * const channel = object;
  const LPC_GPDMACH_Type * const reg = channel->parent.reg;
  struct GpDmaListEntry * const next = (struct GpDmaListEntry *)reg->LLI;

  /* Transfer next chunk and stop */
  if (next)
    next->next = 0;
}
