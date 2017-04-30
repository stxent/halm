/*
 * gpdma_circular.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <malloc.h>
#include <halm/platform/nxp/gpdma_circular.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_READY,
  STATE_DONE,
  STATE_BUSY,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum result);
static void startTransfer(struct GpDmaCircular *, const struct GpDmaEntry *);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);

static void channelSetCallback(void *, void (*)(void *), void *);
static void channelConfigure(void *, const void *);

static enum result channelEnable(void *);
static void channelDisable(void *);
static size_t channelPending(const void *);
static size_t channelResidue(const void *);
static enum result channelStatus(const void *);

static void channelAppend(void *, void *, const void *, size_t);
static void channelClear(void *);
/*----------------------------------------------------------------------------*/
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDmaCircular),
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
const struct DmaClass * const GpDmaCircular = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDmaCircular * const channel = object;

  switch (res)
  {
    case E_OK:
      channel->state = STATE_DONE;
      break;

    case E_ERROR:
      channel->state = STATE_ERROR;
      break;

    default:
      break;
  }

  if (channel->callback)
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
  reg->CONFIG = channel->base.config;

  /* Start the transfer */
  reg->CONFIG |= CONFIG_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaCircularConfig * const config = configBase;
  assert(config);

  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDmaCircular * const channel = object;
  enum result res;

  assert(config->number > 0);

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(4, sizeof(struct GpDmaEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->capacity = config->number;
  channel->queued = 0;
  channel->control = 0;
  channel->state = STATE_IDLE;
  channel->silent = config->silent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct GpDmaCircular * const channel = object;

  free(channel->list);
  GpDmaBase->deinit(channel);
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
    if (channel->callback && (!channel->silent || index < channel->queued - 1))
      channel->list[index].control |= CONTROL_INT;
    else
      channel->list[index].control &= ~CONTROL_INT;
  }
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDmaCircular * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
}
/*----------------------------------------------------------------------------*/
static enum result channelEnable(void *object)
{
  struct GpDmaCircular * const channel = object;

  assert(channel->state == STATE_READY);

  if (gpDmaSetDescriptor(channel->base.number, object) != E_OK)
    return E_BUSY;

  gpDmaSetMux(object);

  const uint32_t request = 1 << channel->base.number;

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
  struct GpDmaCircular * const channel = object;
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
  const struct GpDmaCircular * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static size_t channelResidue(const void *object)
{
  const struct GpDmaCircular * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

    const size_t nextEntry = ((uintptr_t)reg->LLI - (uintptr_t)channel->list)
        / sizeof(struct GpDmaEntry);
    const size_t currentEntry =
        (nextEntry > 0 ? nextEntry : channel->capacity) - 1;

    const size_t initialTransfers =
        CONTROL_SIZE_VALUE(channel->list[currentEntry].control);
    const size_t completedTransfers = CONTROL_SIZE_VALUE(reg->CONTROL);

    return initialTransfers - completedTransfers;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
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

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= GPDMA_MAX_TRANSFER);
  assert(channel->queued < channel->capacity);
  assert(channel->state != STATE_BUSY);

  if (channel->state == STATE_ERROR)
    channel->queued = 0;

  struct GpDmaEntry * const entry = channel->list + channel->queued;
  struct GpDmaEntry *previous = 0;

  if (channel->queued)
    previous = channel->list + (channel->queued - 1);

  entry->source = (uintptr_t)source;
  entry->destination = (uintptr_t)destination;
  entry->control = channel->control | CONTROL_SIZE(size);
  entry->next = (uintptr_t)channel->list;

  if (channel->callback)
    entry->control |= CONTROL_INT;
  if (previous)
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
