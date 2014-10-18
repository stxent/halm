/*
 * gpdma_list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <platform/platform_defs.h>
#include <platform/nxp/gpdma_defs.h>
#include <platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
static enum result appendItem(void *, void *, const void *, uint32_t);
static void clearItemList(void *);
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
  struct GpDmaListItem * const item = channel->list + channel->number;

  if (size > GPDMA_MAX_TRANSFER)
    return E_VALUE;

  if (channel->number++)
  {
    struct GpDmaListItem *previous = item - 1;

    /* Append current element to the previous one */
    previous->next = (uint32_t)item;

    if (channel->silent)
      previous->control &= ~CONTROL_INT;
  }

  item->source = (uint32_t)source;
  item->destination = (uint32_t)destination;
  item->control = channel->parent.control | CONTROL_SIZE(size);
  item->next = channel->circular ? (uint32_t)channel->list : 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void clearItemList(void *object)
{
  ((struct GpDmaList *)object)->number = 0;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDmaList * const channel = object;

  channel->error = res != E_OK && res != E_BUSY;

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
  channel->list = malloc(sizeof(struct GpDmaListItem) * config->size);
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
  channel->error = false;
  channel->number = 0;
  channel->silent = config->silent;
  channel->size = config->size;

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
  const struct GpDmaListItem * const next =
      (const struct GpDmaListItem *)reg->LLI;

  if (!next)
    return 0;

  if (next == channel->list)
    return channel->number - 1;
  else
    return (uint32_t)(next - channel->list) - 1;
}
/*----------------------------------------------------------------------------*/
static enum result channelStart(void *object, void *destination,
    const void *source, uint32_t size)
{
  struct GpDmaList * const channel = object;

  if (size > (uint32_t)(channel->capacity * channel->size))
    return E_VALUE;

  if (gpDmaSetDescriptor(channel->parent.number, object) != E_OK)
    return E_BUSY;

  gpDmaSetupMux(object);
  clearItemList(channel);
  channel->error = false;

  uint32_t chunk, offset = 0;

  while (offset < size)
  {
    chunk = size - offset >= channel->size ? channel->size : size - offset;
    offset += chunk;

    appendItem(channel, destination, source, chunk);

    if (channel->parent.control & CONTROL_DST_INC)
      destination += chunk;
    if (channel->parent.control & CONTROL_SRC_INC)
      source += chunk;
  }

  const struct GpDmaListItem * const first = channel->list;
  const uint32_t request = 1 << channel->parent.number;
  LPC_GPDMACH_Type * const reg = channel->parent.reg;

  reg->SRCADDR = first->source;
  reg->DESTADDR = first->destination;
  reg->CONTROL = first->control;
  reg->LLI = first->next;
  reg->CONFIG = channel->parent.config;

  LPC_GPDMA->INTTCCLEAR |= request;
  LPC_GPDMA->INTERRCLEAR |= request;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDmaList * const channel = object;
  const LPC_GPDMACH_Type * const reg = channel->parent.reg;

  if (channel->error)
    return E_ERROR;

  return gpDmaGetDescriptor(channel->parent.number) == object
      && (reg->CONFIG & CONFIG_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void channelStop(void *object)
{
  LPC_GPDMACH_Type * const reg = ((struct GpDmaList *)object)->parent.reg;

  /* Complete current transfer and stop */
  reg->LLI = 0;
}
