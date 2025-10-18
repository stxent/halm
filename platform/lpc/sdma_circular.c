/*
 * sdma_circular.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sdma_circular.h>
#include <halm/platform/lpc/sdma_defs.h>
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
static size_t getCurrentEntry(const struct SdmaCircular *);
static void interruptHandler(void *, enum Result);
static void startTransfer(struct SdmaCircular *, const struct SdmaEntry *);
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
const struct DmaClass * const SdmaCircular = &(const struct DmaClass){
    .size = sizeof(struct SdmaCircular),
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
static size_t getCurrentEntry(const struct SdmaCircular *channel)
{
  const struct SdmaEntry * const head = channel->base.head;
  const size_t offset = (uintptr_t)head->next - (uintptr_t)channel->list;
  const size_t next = offset / sizeof(struct SdmaEntry);
  const size_t current = (next > 0 ? next : channel->queued) - 1;

  return current;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct SdmaCircular * const channel = object;

  if (res != E_BUSY)
  {
    LPC_SDMA->INTENCLR = 1UL << channel->base.number;
    sdmaResetInstance(channel->base.number);

    channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;
  }

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct SdmaCircular *channel,
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
  const struct SdmaCircularConfig * const config = configBase;
  assert(config != NULL);
  assert(config->number > 0);

  const struct SdmaBaseConfig baseConfig = {
      .request = config->request,
      .trigger = config->trigger,
      .channel = config->channel,
      .priority = config->priority,
      .polarity = config->polarity
  };
  struct SdmaCircular * const channel = object;
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
  channel->queued = 0;
  channel->transferConfig = 0;
  channel->state = STATE_IDLE;
  channel->oneshot = config->oneshot;
  channel->silent = config->silent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct SdmaCircular * const channel = object;

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

  struct SdmaCircular * const channel = object;

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
  struct SdmaCircular * const channel = object;

  assert(channel->state != STATE_BUSY);

  channel->callback = callback;
  channel->callbackArgument = argument;

  for (size_t index = 0; index < channel->queued; ++index)
  {
    if (channel->callback && (!channel->silent || index == channel->queued - 1))
      channel->list[index].config |= XFERCFG_SETINTA;
    else
      channel->list[index].config &= ~XFERCFG_SETINTA;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct SdmaCircular * const channel = object;
  const uint8_t number = channel->base.number;
  const uint32_t mask = 1UL << number;

  assert(channel->state == STATE_READY || channel->state == STATE_DONE);

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
  struct SdmaCircular * const channel = object;

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
  const struct SdmaCircular * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_SDMA_CHANNEL_Type * const reg = channel->base.reg;
    const uint32_t transferConfig = reg->XFERCFG;
    const uint32_t left = XFERCFG_XFERCOUNT_VALUE(transferConfig) + 1;
    const unsigned int width = XFERCFG_WIDTH_VALUE(transferConfig);
    const size_t index = getCurrentEntry(channel);

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
  const struct SdmaCircular * const channel = object;

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
  struct SdmaCircular * const channel = object;
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
  assert(channel->state != STATE_BUSY && channel->state != STATE_READY);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
    channel->queued = 0;

  struct SdmaEntry * const entry = channel->list + channel->queued;
  struct SdmaEntry *previous = NULL;

  if (channel->queued)
    previous = channel->list + (channel->queued - 1);

  entry->source = (uintptr_t)source + (count << width) * srcStride;
  entry->destination = (uintptr_t)destination + (count << width) * dstStride;
  entry->config = transferConfig | XFERCFG_XFERCOUNT(count);

  if (channel->oneshot || channel->callback != NULL)
    entry->config |= XFERCFG_SETINTA;

  if (!channel->oneshot)
  {
    entry->config |= XFERCFG_RELOAD;
    entry->next = (uintptr_t)channel->list;
  }
  else
    entry->next = 0;

  if (previous != NULL)
  {
    if (channel->silent)
      previous->config &= ~XFERCFG_SETINTA;

    /* Link previous element with the new one */
    previous->next = (uintptr_t)entry;
    /* Enable reload in the previous entry when one-shot mode is enabled */
    if (channel->oneshot)
      previous->config |= XFERCFG_RELOAD;
  }

  ++channel->queued;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  struct SdmaCircular * const channel = object;

  channel->queued = 0;
  channel->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct SdmaCircular * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued;

  return 0;
}
