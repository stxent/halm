/*
 * gpdma_list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_defs.h>
#include <halm/platform/lpc/gpdma_list.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <malloc.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_READY,
  STATE_BUSY,
  STATE_DONE,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum Result);
static void startTransfer(struct GpDmaList *, const struct GpDmaEntry *);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);

static void channelSetCallback(void *, void (*)(void *), void *);
static void channelConfigure(void *, const void *);

static enum Result channelEnable(void *);
static void channelDisable(void *);
static size_t channelPending(const void *);
static size_t channelResidue(const void *);
static enum Result channelStatus(const void *);

static void channelAppend(void *, void *, const void *, size_t);
static void channelClear(void *);
/*----------------------------------------------------------------------------*/
const struct DmaClass * const GpDmaList = &(const struct DmaClass){
    .size = sizeof(struct GpDmaList),
    .init = channelInit,
    .deinit = channelDeinit,

    .setCallback = channelSetCallback,
    .configure = channelConfigure,

    .enable = channelEnable,
    .disable = channelDisable,
    .pending = channelPending,
    .residue = channelResidue,
    .status = channelStatus,

    .append = channelAppend,
    .clear = channelClear
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct GpDmaList * const channel = object;
  bool event = false;

  --channel->queued;

  switch (res)
  {
    case E_OK:
      if (channel->queued)
      {
        /* Underrun occurred */
        size_t index = channel->index;

        /* Calculate index of the first stalled chunk */
        if (index >= channel->queued)
          index -= channel->queued;
        else
          index += channel->capacity - channel->queued;

        /* Restart stalled transfer */
        startTransfer(channel, &channel->list[index]);
        event = true;
      }
      else
      {
        channel->state = STATE_DONE;
      }
      break;

    case E_BUSY:
      event = true;
      break;

    default:
      channel->state = STATE_ERROR;
      break;
  }

  if (channel->state != STATE_BUSY)
  {
    gpDmaResetInstance(channel->base.number);
    event = true;
  }

  if (event && channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct GpDmaList *channel,
    const struct GpDmaEntry *entry)
{
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  reg->SRCADDR = entry->source;
  reg->DESTADDR = entry->destination;
  reg->CONTROL = entry->control;
  reg->LLI = entry->next;
  reg->CONFIG = channel->base.config | CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaListConfig * const config = configBase;
  assert(config);

  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDmaList * const channel = object;
  enum Result res;

  assert(config->number > 0);

  /* Call base class constructor */
  if ((res = GpDmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(4, sizeof(struct GpDmaEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->capacity = config->number;
  channel->index = 0;
  channel->queued = 0;
  channel->control = 0;
  channel->state = STATE_IDLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpDmaList * const channel = object;

  free(channel->list);

  if (GpDmaBase->deinit)
    GpDmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDmaList * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDmaList * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
  channel->control |= CONTROL_INT;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct GpDmaList * const channel = object;
  const uint8_t number = channel->base.number;
  const uint32_t request = 1 << number;

  assert(channel->state == STATE_READY);
  if (!gpDmaSetInstance(number, object))
    return E_BUSY;
  gpDmaSetMux(object);

  /* Clear interrupt requests for the current channel */
  LPC_GPDMA->INTTCCLEAR = request;
  LPC_GPDMA->INTERRCLEAR = request;

  /* Start the transfer */
  channel->state = STATE_BUSY;
  startTransfer(channel, &channel->list[0]);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpDmaList * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  if (channel->state == STATE_BUSY)
  {
    reg->CONFIG &= ~CONFIG_ENABLE;
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static size_t channelPending(const void *object)
{
  const struct GpDmaList * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static size_t channelResidue(const void *object)
{
  const struct GpDmaList * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
    size_t index = channel->index + channel->capacity - channel->queued;

    if (index >= channel->capacity)
      index -= channel->capacity;

    const uint32_t transfers = CONTROL_SIZE_VALUE(reg->CONTROL);

    if (reg->LLI == channel->list[index].next)
    {
      /* Linked list item is not changed, transfer count is correct */
      return (size_t)transfers;
    }
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct GpDmaList * const channel = object;

  switch ((enum State)channel->state)
  {
    case STATE_IDLE:
    case STATE_READY:
    case STATE_DONE:
      return E_OK;

    case STATE_BUSY:
      return E_BUSY;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static void channelAppend(void *object, void *destination, const void *source,
    size_t size)
{
  struct GpDmaList * const channel = object;

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= GPDMA_MAX_TRANSFER);
  assert(channel->queued < channel->capacity);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
  {
    channel->index = 0;
    channel->queued = 0;
  }

  struct GpDmaEntry * const entry = channel->list + channel->index;
  struct GpDmaEntry *previous = 0;

  if (channel->queued)
  {
    /* There are initialized entries in the list */
    previous = channel->index ? entry : channel->list + channel->capacity;
    --previous;
  }

  if (++channel->index >= channel->capacity)
    channel->index = 0;

  entry->source = (uintptr_t)source;
  entry->destination = (uintptr_t)destination;
  entry->control = channel->control | CONTROL_SIZE(size);
  entry->next = 0;

  if (previous)
  {
    /* Link previous element with the new one */
    previous->next = (uintptr_t)entry;
  }

  ++channel->queued;

  if (channel->state != STATE_BUSY)
    channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  struct GpDmaList * const channel = object;

  channel->index = 0;
  channel->queued = 0;
  channel->state = STATE_IDLE;
}
