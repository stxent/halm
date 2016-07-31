/*
 * gpdma.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gpdma.h>
#include <halm/platform/nxp/gpdma_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_READY,
  STATE_DONE,
  STATE_BUSY,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum result);
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *, const void *);
static void channelDeinit(void *);

static void channelCallback(void *, void (*)(void *), void *);
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
    .size = sizeof(struct GpDma),
    .init = channelInit,
    .deinit = channelDeinit,

    .callback = channelCallback,
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
const struct DmaClass * const GpDma = &channelTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum result res)
{
  struct GpDma * const channel = object;

  gpDmaClearDescriptor(channel->base.number);
  channel->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (channel->callback)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result channelInit(void *object, const void *configBase)
{
  const struct GpDmaConfig * const config = configBase;
  const struct GpDmaBaseConfig baseConfig = {
      .event = config->event,
      .type = config->type,
      .channel = config->channel
  };
  struct GpDma * const channel = object;
  enum result res;

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
  GpDmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpDma * const channel = object;

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct GpDmaSettings * const settings = settingsBase;
  struct GpDma * const channel = object;

  channel->control = gpDmaBaseCalcControl(object, settings);
  channel->control |= CONTROL_INT;
}
/*----------------------------------------------------------------------------*/
static enum result channelEnable(void *object)
{
  struct GpDma * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  assert(channel->state == STATE_READY);

  const enum result res = gpDmaSetDescriptor(number, object);
  if (res != E_OK)
    return res;

  gpDmaSetMux(object);

  reg->SRCADDR = channel->source;
  reg->DESTADDR = channel->destination;
  reg->CONTROL = channel->control;
  reg->CONFIG = channel->base.config;
  reg->LLI = 0;

  const uint32_t request = 1 << number;

  /* Clear interrupt requests for current channel */
  LPC_GPDMA->INTTCCLEAR = request;
  LPC_GPDMA->INTERRCLEAR = request;

  /* Start the transfer */
  channel->state = STATE_BUSY;
  reg->CONFIG |= CONFIG_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct GpDma * const channel = object;
  LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  if (channel->state == STATE_BUSY)
  {
    reg->CONFIG &= ~CONFIG_ENABLE;
    channel->state = STATE_IDLE;
  }
}
/*----------------------------------------------------------------------------*/
static size_t channelPending(const void *object)
{
  const struct GpDma * const channel = object;

  return channel->state == STATE_BUSY ? 1 : 0;
}
/*----------------------------------------------------------------------------*/
static size_t channelResidue(const void *object)
{
  const struct GpDma * const channel = object;
  const LPC_GPDMA_CHANNEL_Type * const reg = channel->base.reg;

  if (channel->state == STATE_BUSY)
  {
    return CONTROL_SIZE_VALUE(channel->control)
        - CONTROL_SIZE_VALUE(reg->CONTROL);
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static enum result channelStatus(const void *object)
{
  const struct GpDma * const channel = object;

  switch ((enum state)channel->state)
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
  struct GpDma * const channel = object;

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
  struct GpDma * const channel = object;

  channel->state = STATE_IDLE;
}
