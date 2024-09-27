/*
 * gpdma_circular.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_circular.h>
#include <halm/platform/lpc/gpdma_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
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
static size_t getCurrentEntry(const struct GpDmaCircular *);
static void interruptHandler(void *, enum Result);
static void startTransfer(struct GpDmaCircular *, const struct GpDmaEntry *);
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
static void channelDeinit(void *);

static void channelConfigure(void *, const void *);
static void channelSetCallback(void *, void (*)(void *), void *);

static enum Result channelEnable(void *);
static void channelDisable(void *);
static enum Result channelResidue(const void *, size_t *);
static enum Result channelStatus(const void *);

static void channelAppend(void *, void *, const void *, size_t);
static void channelClear(void *);
static size_t channelQueued(const void *);
/*----------------------------------------------------------------------------*/
const struct DmaClass * const GpDmaCircular = &(const struct DmaClass){
    .size = sizeof(struct GpDmaCircular),
    .init = channelInit,
    .deinit = channelDeinit,

    .configure = channelConfigure,
    .setCallback = channelSetCallback,

    .enable = channelEnable,
    .disable = channelDisable,
    .residue = channelResidue,
    .status = channelStatus,

    .append = channelAppend,
    .clear = channelClear,
    .queued = channelQueued
};
/*----------------------------------------------------------------------------*/
static size_t getCurrentEntry(const struct GpDmaCircular *channel)
{
  const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
  const size_t offset = (uintptr_t)reg->LLI - (uintptr_t)channel->list;
  const size_t next = offset / sizeof(struct GpDmaEntry);
  const size_t current = (next > 0 ? next : channel->queued) - 1;

  return current;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct GpDmaCircular * const channel = object;

  if (res != E_BUSY)
  {
    channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;
    gpDmaResetInstance(channel->base.number);
  }

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct GpDmaCircular *channel,
    const struct GpDmaEntry *entry)
{
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  reg->SRCADDR = entry->source;
  reg->DESTADDR = entry->destination;
  reg->CONTROL = entry->control;
  reg->LLI = entry->next;

  __dmb();
  reg->CONFIG = channel->base.config | CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaCircularConfig * const config = configBase;
  assert(config != NULL);
  assert(config->number > 0);

  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDmaCircular * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(4, sizeof(struct GpDmaEntry) * config->number);
  if (channel->list == NULL)
    return E_MEMORY;

  channel->base.handler = interruptHandler;

  channel->callback = NULL;
  channel->capacity = config->number;
  channel->queued = 0;
  channel->control = 0;
  channel->state = STATE_IDLE;
  channel->oneshot = config->oneshot;
  channel->silent = config->silent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpDmaCircular * const channel = object;

  free(channel->list);

  if (GpDmaBase->deinit != NULL)
    GpDmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDmaCircular * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDmaCircular * const channel = object;

  assert(channel->state != STATE_BUSY);

  channel->callback = callback;
  channel->callbackArgument = argument;

  for (size_t index = 0; index < channel->queued; ++index)
  {
    if (channel->callback && (!channel->silent || index == channel->queued - 1))
      channel->list[index].control |= CONTROL_INT;
    else
      channel->list[index].control &= ~CONTROL_INT;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct GpDmaCircular * const channel = object;
  const uint8_t number = channel->base.number;
  const uint32_t mask = 1 << number;

  assert(channel->state == STATE_READY || channel->state == STATE_DONE);

  if (!gpDmaSetInstance(number, object))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  gpDmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  /* Clear interrupt requests for the current channel */
  LPC_GPDMA->INTTCCLEAR = mask;
  LPC_GPDMA->INTERRCLEAR = mask;

  /* Start the transfer */
  startTransfer(channel, &channel->list[0]);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpDmaCircular * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  if (channel->state == STATE_BUSY)
  {
    reg->CONFIG &= ~CONFIG_ENABLE;
    gpDmaResetInstance(channel->base.number);

    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct GpDmaCircular * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
    const uint32_t control = reg->CONTROL;
    const uint32_t transfers = CONTROL_SIZE_VALUE(control);
    const uint32_t width = 1 << CONTROL_DST_WIDTH_VALUE(control);
    const size_t index = getCurrentEntry(channel);

    if (reg->LLI == channel->list[index].next)
    {
      /* Linked list item is not changed, transfer count is correct */
      *count = (size_t)(transfers * width);
      return E_OK;
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct GpDmaCircular * const channel = object;

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
  struct GpDmaCircular * const channel = object;
  const uint32_t control = channel->control;
  const uint32_t transfers = size >> CONTROL_DST_WIDTH_VALUE(control);

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination % (1 << CONTROL_DST_WIDTH_VALUE(control))));
  assert(!(size % (1 << CONTROL_DST_WIDTH_VALUE(control))));
  assert(!((uintptr_t)source % (1 << CONTROL_SRC_WIDTH_VALUE(control))));
  assert(!(size % (1 << CONTROL_SRC_WIDTH_VALUE(control))));
  assert(transfers > 0 && transfers <= GPDMA_MAX_TRANSFER_SIZE);
  assert(channel->queued < channel->capacity);
  assert(channel->state != STATE_BUSY);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
    channel->queued = 0;

  struct GpDmaEntry * const entry = channel->list + channel->queued;
  struct GpDmaEntry *previous = NULL;

  if (channel->queued)
    previous = channel->list + (channel->queued - 1);

  entry->source = (uintptr_t)source;
  entry->destination = (uintptr_t)destination;
  entry->control = control | CONTROL_SIZE(transfers);

  if (channel->callback != NULL || channel->oneshot)
    entry->control |= CONTROL_INT;

  if (!channel->oneshot)
    entry->next = (uintptr_t)channel->list;
  else
    entry->next = 0;

  if (previous != NULL)
  {
    if (channel->silent)
      previous->control &= ~CONTROL_INT;

    /* Link previous element with the new one */
    previous->next = (uintptr_t)entry;
  }

  ++channel->queued;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  struct GpDmaCircular * const channel = object;

  channel->queued = 0;
  channel->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct GpDmaCircular * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued;

  return 0;
}
