/*
 * gpdma_oneshot.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/nxp/gpdma_oneshot.h>
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
static void interruptHandler(void *, enum Result);
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
static const struct DmaClass channelTable = {
    .size = sizeof(struct GpDmaOneShot),
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
const struct DmaClass * const GpDmaOneShot = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct GpDmaOneShot * const channel = object;

  gpDmaResetInstance(channel->base.number);
  channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct GpDmaOneShotConfig * const config = configBase;
  assert(config);

  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDmaOneShot * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = GpDmaBase->init(object, &baseConfig)) != E_OK)
    return res;

  channel->base.handler = interruptHandler;

  channel->callback = 0;
  channel->control = 0;
  channel->state = STATE_IDLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  if (GpDmaBase->deinit)
    GpDmaBase->deinit(object);
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
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDmaOneShot * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
  channel->control |= CONTROL_INT;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct GpDmaOneShot * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;
  const uint32_t request = 1 << number;

  assert(channel->state == STATE_READY);
  if (!gpDmaSetInstance(number, object))
    return E_BUSY;
  gpDmaSetMux(object);

  reg->SRCADDR = channel->source;
  reg->DESTADDR = channel->destination;
  reg->CONTROL = channel->control;
  reg->LLI = 0;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR = request;
  LPC_GPDMA->INTERRCLEAR = request;

  /* Start the transfer */
  channel->state = STATE_BUSY;
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
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static size_t channelPending(const void *object)
{
  const struct GpDmaOneShot * const channel = object;
  return channel->state != STATE_IDLE && channel->state != STATE_READY ? 1 : 0;
}
/*----------------------------------------------------------------------------*/
static size_t channelResidue(const void *object)
{
  const struct GpDmaOneShot * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

    const size_t initialTransfers = CONTROL_SIZE_VALUE(channel->control);
    const size_t completedTransfers = CONTROL_SIZE_VALUE(reg->CONTROL);

    return initialTransfers - completedTransfers;
  }
  else
    return 0;
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

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= GPDMA_MAX_TRANSFER);
  assert(channel->state != STATE_BUSY && channel->state != STATE_READY);

  channel->destination = (uintptr_t)destination;
  channel->source = (uintptr_t)source;
  channel->control = (channel->control & ~CONTROL_SIZE_MASK)
      | CONTROL_SIZE(size);
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct GpDmaOneShot *)object)->state = STATE_IDLE;
}
