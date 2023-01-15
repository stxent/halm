/*
 * pdma_circular.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/pdma_circular.h>
#include <halm/platform/numicro/pdma_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/memory.h>
#include <assert.h>
#include <malloc.h>
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
static inline NM_PDMA_CHANNEL_Type *getChannelReg(const struct PdmaCircular *);
static size_t getCurrentEntry(const struct PdmaCircular *);
static void interruptHandler(void *, enum Result);
static void startTransfer(struct PdmaCircular *, const struct PdmaEntry *);
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
const struct DmaClass * const PdmaCircular = &(const struct DmaClass){
    .size = sizeof(struct PdmaCircular),
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
static inline NM_PDMA_CHANNEL_Type *getChannelReg(
    const struct PdmaCircular *channel)
{
  NM_PDMA_Type * const reg = channel->base.reg;
  return &reg->CHANNELS[channel->base.number];
}
/*----------------------------------------------------------------------------*/
static size_t getCurrentEntry(const struct PdmaCircular *channel)
{
  const NM_PDMA_CHANNEL_Type * const entry = getChannelReg(channel);
  const uint32_t opmode = DSCT_CTL_OPMODE_VALUE(entry->CTL);
  const uint32_t next = DSCT_NEXT_EXENEXT_TO_ADDRESS(entry->NEXT);

  if (next)
  {
    const uint32_t head = DSCT_NEXT_ADDRESS_TO_NEXT((uintptr_t)channel->list);
    size_t index = (next - head) / sizeof(struct PdmaEntry);

    if (opmode == OPMODE_ACTIVE)
      index = index > 0 ? index - 1 : channel->queued - 1;

    assert(index < channel->queued);
    return index;
  }
  else
    return channel->queued;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct PdmaCircular * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  if (res != E_BUSY)
  {
    reg->CHCTL &= ~(1 << number);
    pdmaResetInstance(number);

    channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;
  }

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startTransfer(struct PdmaCircular *channel,
    const struct PdmaEntry *current)
{
  NM_PDMA_Type * const reg = channel->base.reg;
  NM_PDMA_CHANNEL_Type * const entry = getChannelReg(channel);

  entry->CTL = DSCT_CTL_OPMODE(OPMODE_LIST);
  entry->SA = 0;
  entry->DA = 0;
  entry->NEXT = DSCT_NEXT_ADDRESS_TO_NEXT((uintptr_t)current);

  /* Start the transfer */
  __dsb();
  reg->CHCTL |= 1 << channel->base.number;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PdmaCircularConfig * const config = configBase;
  assert(config);
  assert(config->number > 0);

  const struct PdmaBaseConfig baseConfig = {
      .event = config->event,
      .channel = config->channel
  };
  struct PdmaCircular * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PdmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(4, sizeof(struct PdmaEntry) * config->number);
  if (!channel->list)
    return E_MEMORY;

  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->capacity = config->number;
  channel->queued = 0;
  channel->state = STATE_IDLE;
  channel->fixed = true;
  channel->oneshot = config->oneshot;
  channel->silent = config->silent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct PdmaCircular * const channel = object;

  free(channel->list);
  PdmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct PdmaSettings * const settings = settingsBase;
  struct PdmaCircular * const channel = object;

  assert(settings->burst <= DMA_BURST_128);
  assert(settings->width <= DMA_WIDTH_WORD);

  /* Burst mode is not allowed for single transfers */
  assert(!(channel->base.control & DSCT_CTL_TXTYPE)
      || settings->burst == DMA_BURST_1);

  uint32_t control = channel->base.control & ~(
          DSCT_CTL_BURSIZE_MASK
          | DSCT_CTL_SAINC_MASK
          | DSCT_CTL_DAINC_MASK
          | DSCT_CTL_TXWIDTH_MASK
      );

  control |= DSCT_CTL_BURSIZE(settings->burst);
  control |= DSCT_CTL_TXWIDTH(settings->width);

  if (!settings->source.increment)
    control |= DSCT_CTL_SAINC(ADDRESS_FIX);
  if (!settings->destination.increment)
    control |= DSCT_CTL_DAINC(ADDRESS_FIX);

  channel->base.control = control;
  channel->fixed = (settings->priority == DMA_PRIORITY_FIXED);
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PdmaCircular * const channel = object;

  assert(channel->state != STATE_BUSY);

  channel->callback = callback;
  channel->callbackArgument = argument;

  for (size_t index = 0; index < channel->queued; ++index)
  {
    if (channel->callback && (!channel->silent || index == channel->queued - 1))
      channel->list[index].control &= ~DSCT_CTL_TBINTDIS;
    else
      channel->list[index].control |= DSCT_CTL_TBINTDIS;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct PdmaCircular * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint32_t mask = 1 << channel->base.number;

  assert(channel->state == STATE_READY || channel->state == STATE_DONE);

  if (!pdmaSetInstance(channel->base.number, object))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  pdmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  if (channel->fixed)
    reg->PRISET = mask;
  else
    reg->PRICLR = mask;

  /* Start the transfer */
  startTransfer(channel, &channel->list[0]);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct PdmaCircular * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;

  /*
   * Incorrect sequence of calls: channel should not be disabled when
   * it is not initialized and started.
   */
  assert(channel->state != STATE_IDLE && channel->state != STATE_READY);

  if (channel->state == STATE_BUSY)
  {
    // TODO Clear CHRST at enable?
    reg->CHRST |= 1 << channel->base.number;
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct PdmaCircular * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const NM_PDMA_CHANNEL_Type * const entry = getChannelReg(channel);
    const size_t index = getCurrentEntry(channel);

    const struct PdmaEntry * const current = channel->list + index;
    const size_t transfers = DSCT_CTL_TXCNT_VALUE(entry->CTL) + 1;

    if (DSCT_NEXT_EXENEXT_TO_ADDRESS(entry->NEXT) == current->next)
    {
      /* Linked list item is not changed, transfer count is correct */
      *count = transfers;
      return E_OK;
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct PdmaCircular * const channel = object;

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
  struct PdmaCircular * const channel = object;

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= PDMA_MAX_TRANSFER_SIZE);
  assert(channel->queued < channel->capacity);
  assert(channel->state != STATE_BUSY);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
    channel->queued = 0;

  struct PdmaEntry * const current = channel->list + channel->queued;
  struct PdmaEntry *previous = 0;

  if (channel->queued)
    previous = channel->list + (channel->queued - 1);

  current->source = (uintptr_t)source;
  current->destination = (uintptr_t)destination;
  current->control = channel->base.control | DSCT_CTL_TXCNT(size - 1);

  if (!channel->callback && !channel->oneshot)
    current->control |= DSCT_CTL_TBINTDIS;

  if (channel->oneshot)
  {
    current->control |= DSCT_CTL_OPMODE(OPMODE_BASIC);
    current->next = 0;
  }
  else
  {
    current->control |= DSCT_CTL_OPMODE(OPMODE_LIST);
    current->next = DSCT_NEXT_ADDRESS_TO_NEXT((uintptr_t)channel->list);
  }

  if (previous)
  {
    if (channel->silent)
      previous->control |= DSCT_CTL_TBINTDIS;

    if (channel->oneshot)
    {
      /* Change mode of the previous element from basic to scatter-gather */
      previous->control &= ~DSCT_CTL_OPMODE_MASK;
      previous->control |= DSCT_CTL_OPMODE(OPMODE_LIST);
    }

    /* Link previous element with the new one */
    previous->next = DSCT_NEXT_ADDRESS_TO_NEXT((uintptr_t)current);
  }

  ++channel->queued;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct PdmaCircular *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct PdmaCircular * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued - getCurrentEntry(channel);

  return 0;
}
