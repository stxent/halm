/*
 * sdma_oneshot.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sdma_defs.h>
#include <halm/platform/lpc/sdma_oneshot.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
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
const struct DmaClass * const SdmaOneShot = &(const struct DmaClass){
    .size = sizeof(struct SdmaOneShot),
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
  struct SdmaOneShot * const channel = object;
  const uint32_t mask = 1UL << channel->base.number;

  LPC_SDMA->ENABLECLR = mask;
  LPC_SDMA->INTENCLR = mask;
  sdmaResetInstance(channel->base.number);

  channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct SdmaOneShotConfig * const config = configBase;
  assert(config != NULL);

  const struct SdmaBaseConfig baseConfig = {
      .request = config->request,
      .trigger = config->trigger,
      .channel = config->channel,
      .priority = config->priority,
      .polarity = config->polarity
  };
  struct SdmaOneShot * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SdmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->base.handler = interruptHandler;

  channel->callback = NULL;
  channel->transferConfig = 0;
  channel->state = STATE_IDLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  if (SdmaBase->deinit != NULL)
    SdmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct SdmaSettings * const settings = settingsBase;
  assert(settings->burst <= DMA_BURST_1024);
  assert(settings->width <= DMA_WIDTH_WORD);

  struct SdmaOneShot * const channel = object;

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
  struct SdmaOneShot * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct SdmaOneShot * const channel = object;
  LPC_SDMA_CHANNEL_Type * const reg = channel->base.reg;
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

  channel->base.head->config = 0;
  channel->base.head->source = channel->source;
  channel->base.head->destination = channel->destination;
  channel->base.head->next = 0;
  reg->CFG = channel->base.config;
  reg->XFERCFG = channel->transferConfig;

  /* Clear pending interrupt requests */
  LPC_SDMA->INTA = mask;
  LPC_SDMA->ERRINT = mask;
  /* Enable interrupts */
  LPC_SDMA->INTENSET = mask;

  /* Start the transfer */
  __dmb();
  LPC_SDMA->ENABLESET = mask;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct SdmaOneShot * const channel = object;

  if (channel->state == STATE_BUSY)
  {
    const uint32_t number = channel->base.number;
    const uint32_t mask = 1UL << number;

    LPC_SDMA->ENABLECLR = mask;
    LPC_SDMA->INTENCLR = mask;
    while (LPC_SDMA->BUSY & mask);
    LPC_SDMA->ABORT = mask;

    sdmaResetInstance(number);
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct SdmaOneShot * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_SDMA_CHANNEL_Type * const reg = channel->base.reg;
    const uint32_t transferConfig = reg->XFERCFG;
    const uint32_t left = XFERCFG_XFERCOUNT_VALUE(transferConfig) + 1;
    const unsigned int width = XFERCFG_WIDTH_VALUE(transferConfig);

    *count = (size_t)(left << width);
    return E_OK;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct SdmaOneShot * const channel = object;

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
  struct SdmaOneShot * const channel = object;

  const uint32_t transferConfig =
      channel->transferConfig & ~XFERCFG_XFERCOUNT_MASK;
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
  assert(channel->state != STATE_BUSY && channel->state != STATE_READY);

  channel->source = (uintptr_t)source + (count << width) * srcStride;
  channel->destination = (uintptr_t)destination + (count << width) * dstStride;
  channel->transferConfig = transferConfig | XFERCFG_XFERCOUNT(count);
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct SdmaOneShot *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct SdmaOneShot * const channel = object;
  return channel->state != STATE_IDLE && channel->state != STATE_READY ? 1 : 0;
}
