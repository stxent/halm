/*
 * gpdma_oneshot.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_defs.h>
#include <halm/platform/lpc/gpdma_oneshot.h>
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
const struct DmaClass * const GpDmaOneShot = &(const struct DmaClass){
    .size = sizeof(struct GpDmaOneShot),
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
  struct GpDmaOneShot * const channel = object;

  gpDmaResetInstance(channel->base.number);
  channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaOneShotConfig * const config = configBase;
  assert(config != NULL);

  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDmaOneShot * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->base.handler = interruptHandler;

  channel->callback = NULL;
  channel->control = 0;
  channel->state = STATE_IDLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  if (GpDmaBase->deinit != NULL)
    GpDmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDmaOneShot * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
  channel->control |= CONTROL_INT;
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDmaOneShot * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct GpDmaOneShot * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;
  const uint32_t mask = 1 << number;

  assert(channel->state == STATE_READY);

  if (!gpDmaSetInstance(number, object))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  gpDmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  reg->SRCADDR = channel->source;
  reg->DESTADDR = channel->destination;
  reg->CONTROL = channel->control;
  reg->LLI = 0;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR = mask;
  LPC_GPDMA->INTERRCLEAR = mask;

  /* Start the transfer */
  __dmb();
  reg->CONFIG = channel->base.config | CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpDmaOneShot * const channel = object;
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
  const struct GpDmaOneShot * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
    const uint32_t control = reg->CONTROL;
    const uint32_t transfers = CONTROL_SIZE_VALUE(control);
    const uint32_t width = 1 << CONTROL_DST_WIDTH_VALUE(control);

    *count = (size_t)(transfers * width);
    return E_OK;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct GpDmaOneShot * const channel = object;

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
  struct GpDmaOneShot * const channel = object;
  const uint32_t control = channel->control;
  const uint32_t transfers = size >> CONTROL_DST_WIDTH_VALUE(control);

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination % (1 << CONTROL_DST_WIDTH_VALUE(control))));
  assert(!(size % (1 << CONTROL_DST_WIDTH_VALUE(control))));
  assert(!((uintptr_t)source % (1 << CONTROL_SRC_WIDTH_VALUE(control))));
  assert(!(size % (1 << CONTROL_SRC_WIDTH_VALUE(control))));
  assert(transfers > 0 && transfers <= GPDMA_MAX_TRANSFER_SIZE);
  assert(channel->state != STATE_BUSY && channel->state != STATE_READY);

  channel->destination = (uintptr_t)destination;
  channel->source = (uintptr_t)source;
  channel->control = (control & ~CONTROL_SIZE_MASK) | CONTROL_SIZE(transfers);
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct GpDmaOneShot *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct GpDmaOneShot * const channel = object;
  return channel->state != STATE_IDLE && channel->state != STATE_READY ? 1 : 0;
}
