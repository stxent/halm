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
static enum result appendItem(void *, void *, const void *, uint32_t);
static void clearEntryList(void *);
static void interruptHandler(void *, enum result);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);
static void channelCallback(void *, void (*)(void *), void *);
static uint32_t channelIndex(const void *);
static enum result channelStart(void *, void *, const void *, uint32_t);
static enum result channelStatus(const void *);
static void channelStop(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDmaList),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
    .index = channelIndex,
    .start = channelStart,
    .status = channelStatus,
    .stop = channelStop
};
/*----------------------------------------------------------------------------*/
const struct DmaClass * const GpDmaList = &channelTable;
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *object, void *destination,
    const void *source, uint32_t size)
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

  if (channel->circular)
  {
    if (channel->unused)
    {
      /* Entry is not initialized yet */
      entry->next = (uint32_t)channel->list;
    }
  }
  else
    entry->next = 0;

  if (channel->unused)
    --channel->unused;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clearEntryList(void *object)
{
  struct GpDmaList * const channel = object;

  channel->unused = channel->capacity;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDmaList * const channel = object;

  channel->last = (res == E_OK || res == E_BUSY) ? E_OK : res;

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

  if (!config->number)
    return E_VALUE;

  if (!config->size || config->size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

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
  channel->circular = config->circular;
  channel->current = 0;
  channel->last = E_OK;
  channel->silent = config->silent;
  channel->size = config->size;
  channel->unused = config->number; /* Similar as capacity */

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
static uint32_t channelIndex(const void *object)
{
  const struct GpDmaList * const channel = object;
  const LPC_GPDMACH_Type * const reg = channel->parent.reg;
  const struct GpDmaListEntry * const next =
      (const struct GpDmaListEntry *)reg->LLI;

  if (!next)
  {
    return (channel->current ? channel->current : channel->capacity) - 1;
  }
  else if (next == channel->list)
  {
    return channel->capacity - 1;
  }
  else
  {
    return (uint32_t)(next - channel->list) - 1;
  }
}
/*----------------------------------------------------------------------------*/
static enum result channelStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDmaList * const channel = object;
  LPC_GPDMACH_Type * const reg = channel->parent.reg;
  const bool active = gpDmaGetDescriptor(channel->parent.number) == object;

  assert(size && size <= (uint32_t)(channel->capacity * channel->size));

  if (!active)
  {
    if (gpDmaSetDescriptor(channel->parent.number, object) != E_OK)
      return E_BUSY;

    gpDmaSetMux(object);
    clearEntryList(channel);
    channel->last = E_OK;
  }

  uint32_t offset = 0;

  while (offset < size)
  {
    const uint32_t chunk = size - offset >= channel->size ?
        channel->size : size - offset;

    offset += chunk;
    appendItem(channel, destination, source, chunk);

    if (channel->parent.control & CONTROL_DST_INC)
      destination = (void *)((uint32_t)destination + chunk);
    if (channel->parent.control & CONTROL_SRC_INC)
      source = (const void *)((uint32_t)source + chunk);
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
    if (!reg->LLI || !(reg->CONFIG & CONFIG_ENABLE))
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
  const LPC_GPDMACH_Type * const reg = channel->parent.reg;

  if (channel->last != E_OK)
    return channel->last;

  return gpDmaGetDescriptor(channel->parent.number) == object
      && (reg->CONFIG & CONFIG_ENABLE) ? E_BUSY : E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  LPC_GPDMACH_Type * const reg = ((struct GpDmaList *)object)->parent.reg;

  /* Complete current transfer and stop */
  reg->LLI = 0;
}
