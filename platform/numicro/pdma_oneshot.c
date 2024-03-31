/*
 * pdma_oneshot.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/pdma_defs.h>
#include <halm/platform/numicro/pdma_oneshot.h>
#include <halm/platform/platform_defs.h>
#include <xcore/memory.h>
#include <assert.h>
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
const struct DmaClass * const PdmaOneShot = &(const struct DmaClass){
    .size = sizeof(struct PdmaOneShot),
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
  struct PdmaOneShot * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  reg->CHCTL &= ~CHCTL_CH(number);
  pdmaUnbindInstance(&channel->base);

  channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PdmaOneShotConfig * const config = configBase;
  assert(config != NULL);

  const struct PdmaBaseConfig baseConfig = {
      .event = config->event,
      .channel = config->channel
  };
  struct PdmaOneShot * const channel = object;

  /* Call base class constructor */
  const enum Result res = PdmaBase->init(channel, &baseConfig);

  if (res == E_OK)
  {
    channel->base.control |= DSCT_CTL_OPMODE(OPMODE_BASIC);
    channel->base.handler = interruptHandler;
    channel->callback = NULL;
    channel->state = STATE_IDLE;
    channel->fixed = true;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  PdmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct PdmaSettings * const settings = settingsBase;
  struct PdmaOneShot * const channel = object;

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
  struct PdmaOneShot * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct PdmaOneShot * const channel = object;
  const uint8_t number = channel->base.number;

  NM_PDMA_Type * const reg = channel->base.reg;
  const uint32_t mask = 1 << number;

  assert(channel->state == STATE_READY);

  if (!pdmaBindInstance(&channel->base))
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
  pdmaStartTransfer(&channel->base,
      channel->base.control | DSCT_CTL_TXCNT(channel->transfers - 1),
      channel->source, channel->destination, 0);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct PdmaOneShot * const channel = object;
  NM_PDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  if (channel->state == STATE_BUSY)
  {
    reg->CHRST = CHRST_CH(number);
    pdmaUnbindInstance(&channel->base);

    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct PdmaOneShot * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const NM_PDMA_Type * const reg = channel->base.reg;
    const NM_PDMA_CHANNEL_Type * const entry =
        &reg->CHANNELS[channel->base.number];

    const uint32_t control = entry->CTL;
    const uint32_t transfers = DSCT_CTL_TXCNT_VALUE(control) + 1;
    const uint32_t width = DSCT_CTL_TXWIDTH_VALUE(control);

    *count = (size_t)(transfers * width);
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct PdmaOneShot * const channel = object;

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
  struct PdmaOneShot * const channel = object;
  const uint32_t control = channel->base.control;
  const uint32_t transfers = size >> DSCT_CTL_TXWIDTH_VALUE(control);

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(!((uintptr_t)source % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(!(size % (1 << DSCT_CTL_TXWIDTH_VALUE(control))));
  assert(transfers > 0 && transfers <= PDMA_MAX_TRANSFER_SIZE);
  assert(channel->state != STATE_BUSY && channel->state != STATE_READY);

  channel->destination = (uintptr_t)destination;
  channel->source = (uintptr_t)source;
  channel->transfers = (uint16_t)transfers;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct PdmaOneShot *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct PdmaOneShot * const channel = object;
  return channel->state != STATE_IDLE && channel->state != STATE_READY ? 1 : 0;
}
