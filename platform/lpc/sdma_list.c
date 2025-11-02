/*
 * sdma_list.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sdma_defs.h>
#include <halm/platform/lpc/sdma_list.h>
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
static void interruptHandler(void *, enum Result);
static void startTransfer(struct SdmaList *, const struct SdmaEntry *);
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
const struct DmaClass * const SdmaList = &(const struct DmaClass){
    .size = sizeof(struct SdmaList),
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
static void interruptHandler(void *object, enum Result res)
{
  struct SdmaList * const channel = object;
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
    LPC_SDMA->INTENCLR = 1UL << channel->base.number;
    sdmaResetInstance(channel->base.number);

    event = true;
  }

  if (event && channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct SdmaList *channel,
    const struct SdmaEntry *entry)
{
  LPC_SDMA_CHANNEL_Type * const reg = channel->base.reg;

  channel->base.head->config = entry->config;
  channel->base.head->source = entry->source;
  channel->base.head->destination = entry->destination;
  channel->base.head->next = entry->next;
  reg->CFG = channel->base.config;
  reg->XFERCFG = entry->config;

  /* Start the transfer */
  __dmb();
  LPC_SDMA->ENABLESET = 1UL << channel->base.number;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct SdmaListConfig * const config = configBase;
  assert(config != NULL);
  assert(config->number > 0);

  const struct SdmaBaseConfig baseConfig = {
      .request = config->request,
      .trigger = config->trigger,
      .channel = config->channel,
      .priority = config->priority,
      .polarity = config->polarity
  };
  struct SdmaList * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SdmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(sizeof(struct SdmaEntry),
      sizeof(struct SdmaEntry) * config->number);
  if (channel->list == NULL)
    return E_MEMORY;

  channel->base.handler = interruptHandler;

  channel->callback = NULL;
  channel->capacity = config->number;
  channel->index = 0;
  channel->queued = 0;
  channel->transferConfig = 0;
  channel->state = STATE_IDLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct SdmaList * const channel = object;

  free(channel->list);

  if (SdmaBase->deinit != NULL)
    SdmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct SdmaSettings * const settings = settingsBase;
  assert(settings->burst <= DMA_BURST_1024);
  assert(settings->width <= DMA_WIDTH_WORD);

  struct SdmaList * const channel = object;

  channel->base.config &=
      ~(CFG_BURSTPOWER_MASK | CFG_SRCBURSTWRAP | CFG_DSTBURSTWRAP);

  channel->base.config |= CFG_BURSTPOWER(settings->burst);
  if (settings->source.wrap)
    channel->base.config |= CFG_SRCBURSTWRAP;
  if (settings->destination.wrap)
    channel->base.config |= CFG_DSTBURSTWRAP;

  channel->transferConfig = sdmaBaseCalcTransferConfig(object, settings);
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SdmaList * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct SdmaList * const channel = object;
  const uint8_t number = channel->base.number;
  const uint32_t mask = 1UL << number;

  assert(channel->state == STATE_READY);

  if (!sdmaSetInstance(number, object))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  sdmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  /* Clear pending interrupt requests */
  LPC_SDMA->INTA = mask;
  LPC_SDMA->ERRINT = mask;
  /* Enable interrupts */
  LPC_SDMA->INTENSET = mask;

  /* Start the transfer */
  startTransfer(channel, &channel->list[0]);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct SdmaList * const channel = object;

  if (channel->state == STATE_BUSY)
  {
    const uint32_t number = channel->base.number;
    const uint32_t mask = 1UL << number;

    LPC_SDMA->INTENCLR = mask;
    LPC_SDMA->ENABLECLR = mask;
    while (LPC_SDMA->BUSY & mask);
    LPC_SDMA->ABORT = mask;

    sdmaResetInstance(number);
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct SdmaList * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_SDMA_CHANNEL_Type * const reg = channel->base.reg;
    size_t index = channel->index + channel->capacity - channel->queued;

    if (index >= channel->capacity)
      index -= channel->capacity;

    const uint32_t transferConfig = reg->XFERCFG;
    const uint32_t left = XFERCFG_XFERCOUNT_VALUE(transferConfig) + 1;
    const unsigned int width = XFERCFG_WIDTH_VALUE(transferConfig);

    if (channel->base.head->next == channel->list[index].next)
    {
      /* Linked list item is not changed, transfer count is correct */
      *count = (size_t)(left << width);
      return E_OK;
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct SdmaList * const channel = object;

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
  struct SdmaList * const channel = object;
  const uint32_t transferConfig = channel->transferConfig;
  const unsigned int dstStride =
      (1 << XFERCFG_DSTINC_VALUE(transferConfig)) >> 1;
  const unsigned int srcStride =
      (1 << XFERCFG_SRCINC_VALUE(transferConfig)) >> 1;
  const unsigned int width = XFERCFG_WIDTH_VALUE(transferConfig);
  const uint32_t count = (size >> width) - 1;

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination % (1 << width)));
  assert(!((uintptr_t)source % (1 << width)));
  assert(count <= SDMA_MAX_TRANSFER_SIZE && ((count + 1) << width) == size);
  assert(channel->queued < channel->capacity);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
  {
    channel->index = 0;
    channel->queued = 0;
  }

  struct SdmaEntry * const entry = channel->list + channel->index;
  struct SdmaEntry *previous = NULL;

  if (channel->queued)
  {
    /* There are initialized entries in the list */
    previous = channel->index ? entry : channel->list + channel->capacity;
    --previous;
  }

  if (++channel->index >= channel->capacity)
    channel->index = 0;

  entry->source = (uintptr_t)source + (count << width) * srcStride;
  entry->destination = (uintptr_t)destination + (count << width) * dstStride;
  entry->config = transferConfig | XFERCFG_XFERCOUNT(count);
  entry->next = 0;

  if (previous != NULL)
  {
    /* Link previous descriptor with the new one */
    previous->next = (uintptr_t)entry;
    /* Enable descriptor reload */
    previous->config |= XFERCFG_RELOAD;
  }

  ++channel->queued;

  if (channel->state != STATE_BUSY)
    channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  struct SdmaList * const channel = object;

  channel->index = 0;
  channel->queued = 0;
  channel->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct SdmaList * const channel = object;

  if (channel->state != STATE_IDLE)
    return channel->queued;
  return 0;
}
