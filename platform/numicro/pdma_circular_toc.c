/*
 * pdma_circular_toc.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/pdma_circular_toc.h>
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
  STATE_ERROR,
  STATE_TIMEOUT
};
/*----------------------------------------------------------------------------*/
static inline NM_PDMA_CHANNEL_Type *getChannelReg(
    const struct PdmaCircularTOC *);
static size_t getCurrentEntry(const struct PdmaCircularTOC *);
static void interruptHandler(void *, enum Result);
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
const struct DmaClass * const PdmaCircularTOC = &(const struct DmaClass){
    .size = sizeof(struct PdmaCircularTOC),
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
    const struct PdmaCircularTOC *channel)
{
  NM_PDMA_Type * const reg = channel->base.reg;
  return &reg->CHANNELS[channel->base.number];
}
/*----------------------------------------------------------------------------*/
static size_t getCurrentEntry(const struct PdmaCircularTOC *channel)
{
  const NM_PDMA_CHANNEL_Type * const entry = getChannelReg(channel);
  const uint32_t opmode = DSCT_CTL_OPMODE_VALUE(entry->CTL);
  const uint32_t next = DSCT_NEXT_EXENEXT_VALUE(entry->NEXT);

  if (next)
  {
    const uint32_t head = DSCT_NEXT_NEXT((uintptr_t)channel->list);
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
  struct PdmaCircularTOC * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  if (res == E_TIMEOUT)
  {
    const uint32_t mask = 1 << number;

    /* Stop channel and wait for pending transfers to be completed */
    reg->PAUSE = mask;
    while (reg->TACTSTS & mask);

    /* Re-enable Timeout Counter, it is disabled in base ISR handler */
    reg->TOUTEN |= TOUTEN_CH(number);
    /* Start the channel again */
    reg->CHCTL |= mask;

    channel->state = STATE_TIMEOUT;
  }
  else if (res == E_BUSY)
  {
    channel->state = STATE_BUSY;
  }
  else
  {
    reg->CHCTL &= ~CHCTL_CH(number);
    reg->TOUTEN &= ~TOUTEN_CH(number);
    reg->TOUTIEN &= ~TOUTIEN_CH(number);

    pdmaUnbindInstance(&channel->base);

    if (res == E_OK)
      channel->state = STATE_DONE;
    else
      channel->state = STATE_ERROR;
  }

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PdmaCircularTOCConfig * const config = configBase;
  assert(config != NULL);
  assert(config->channel < 2);
  assert(config->number > 0);
  assert(config->timeout >= (1UL << 8) && config->timeout < (1UL << 31));

  const struct PdmaBaseConfig baseConfig = {
      .event = config->event,
      .channel = config->channel
  };
  struct PdmaCircularTOC * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = PdmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->list = memalign(4, sizeof(struct PdmaEntry) * config->number);
  if (channel->list == NULL)
    return E_MEMORY;

  uint32_t timeout = config->timeout >> 8;
  uint8_t prescaler = 8;

  while (prescaler < 15 && timeout > 65535)
  {
    timeout >>= 1;
    ++prescaler;
  }

  channel->base.handler = interruptHandler;
  channel->callback = NULL;
  channel->capacity = config->number;
  channel->queued = 0;
  channel->timeout = timeout;
  channel->prescaler = prescaler;
  channel->state = STATE_IDLE;
  channel->fixed = true;
  channel->oneshot = config->oneshot;
  channel->silent = config->silent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  struct PdmaCircularTOC * const channel = object;

  free(channel->list);
  PdmaBase->deinit(channel);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct PdmaSettings * const settings = settingsBase;
  struct PdmaCircularTOC * const channel = object;

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
  struct PdmaCircularTOC * const channel = object;

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
  struct PdmaCircularTOC * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;
  const uint32_t mask = 1 << number;

  assert(channel->state == STATE_READY || channel->state == STATE_DONE);

  if (!pdmaBindInstance(&channel->base))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  reg->TOUTPSC = (reg->TOUTPSC & ~TOUTPSC_CH_MASK(number))
      | TOUTPSC_CH(number, channel->prescaler - 8);
  reg->TOC0_1 = (reg->TOC0_1 & ~TOC_CH_MASK(number))
      | TOC_CH(number, channel->prescaler);
  reg->TOUTIEN |= TOUTIEN_CH(number);
  reg->TOUTEN |= TOUTEN_CH(number);

  pdmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  if (channel->fixed)
    reg->PRISET = mask;
  else
    reg->PRICLR = mask;

  /* Start the transfer */
  pdmaStartTransfer(&channel->base, DSCT_CTL_OPMODE(OPMODE_LIST),
      0, 0, (uintptr_t)&channel->list[0]);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct PdmaCircularTOC * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  if (channel->state == STATE_BUSY)
  {
    pdmaResetChannel(&channel->base);
    reg->TOUTEN &= ~TOUTEN_CH(number);
    reg->TOUTIEN &= ~TOUTIEN_CH(number);
    pdmaUnbindInstance(&channel->base);

    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct PdmaCircularTOC * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const NM_PDMA_CHANNEL_Type * const entry = getChannelReg(channel);
    const size_t index = getCurrentEntry(channel);

    const struct PdmaEntry * const current = channel->list + index;
    const uint32_t next = current->next;

    /*
     * Double check of the next descriptor address is required
     * before and after transfer count reading.
     */
    if (DSCT_NEXT_EXENEXT_VALUE(entry->NEXT) == next)
    {
      const uint32_t control = entry->CTL;
      const uint32_t transfers = DSCT_CTL_TXCNT_VALUE(control) + 1;
      const uint32_t width = DSCT_CTL_TXWIDTH_VALUE(control);

      if (DSCT_NEXT_EXENEXT_VALUE(entry->NEXT) == next)
      {
        /* Linked list item is not changed, transfer count is correct */
        *count = (size_t)(transfers * width);
        return E_OK;
      }
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct PdmaCircularTOC * const channel = object;

  switch ((enum State)channel->state)
  {
    case STATE_IDLE:
    case STATE_READY:
    case STATE_DONE:
      return E_OK;

    case STATE_BUSY:
      return E_BUSY;

    case STATE_TIMEOUT:
      return E_TIMEOUT;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static void channelAppend(void *object, void *destination, const void *source,
    size_t size)
{
  struct PdmaCircularTOC * const channel = object;
  const uint32_t control = channel->base.control;
  const uint32_t transfers = size >> DSCT_CTL_TXWIDTH_VALUE(control);

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(!((uintptr_t)source % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(!(size % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(transfers > 0 && transfers <= PDMA_MAX_TRANSFER_SIZE);
  assert(channel->queued < channel->capacity);
  assert(channel->state != STATE_BUSY);

  if (channel->state == STATE_DONE || channel->state == STATE_ERROR)
    channel->queued = 0;

  struct PdmaEntry * const current = channel->list + channel->queued;
  struct PdmaEntry *previous = NULL;

  if (channel->queued)
    previous = channel->list + (channel->queued - 1);

  current->source = (uintptr_t)source;
  current->destination = (uintptr_t)destination;
  current->control = control | DSCT_CTL_TXCNT(transfers - 1);

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
    current->next = DSCT_NEXT_NEXT((uintptr_t)channel->list);
  }

  if (previous != NULL)
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
    previous->next = DSCT_NEXT_NEXT((uintptr_t)current);
  }

  ++channel->queued;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct PdmaCircularTOC *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct PdmaCircularTOC * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
    return channel->queued - getCurrentEntry(channel);

  return 0;
}
