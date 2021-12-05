/*
 * dma_circular.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_circular.h>
#include <halm/platform/stm32/dma_defs.h>
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
static enum Result streamInit(void *, const void *);
static void streamDeinit(void *);

static void streamConfigure(void *, const void *);
static void streamSetCallback(void *, void (*)(void *), void *);

static enum Result streamEnable(void *);
static void streamDisable(void *);
static enum Result streamResidue(const void *, size_t *);
static enum Result streamStatus(const void *);

static void streamAppend(void *, void *, const void *, size_t);
static void streamClear(void *);
static size_t streamQueued(const void *);
/*----------------------------------------------------------------------------*/
const struct DmaClass * const DmaCircular = &(const struct DmaClass){
    .size = sizeof(struct DmaCircular),
    .init = streamInit,
    .deinit = streamDeinit,

    .configure = streamConfigure,
    .setCallback = streamSetCallback,

    .enable = streamEnable,
    .disable = streamDisable,
    .residue = streamResidue,
    .status = streamStatus,

    .append = streamAppend,
    .clear = streamClear,
    .queued = streamQueued
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object, enum Result res)
{
  struct DmaCircular * const stream = object;

  if (res == E_ERROR)
  {
    STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

    reg->CCR = 0;
    dmaResetInstance(stream->base.number);

    stream->state = STATE_ERROR;
  }

  if (stream->callback)
    stream->callback(stream->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct DmaCircularConfig * const config = configBase;
  assert(config);

  const struct DmaBaseConfig baseConfig = {
      .type = config->type,
      .stream = config->stream,
      .priority = config->priority
  };
  struct DmaCircular * const stream = object;

  /* Call base class constructor */
  const enum Result res = DmaBase->init(stream, &baseConfig);

  if (res == E_OK)
  {
    stream->base.config |= CCR_TCIE | CCR_HTIE | CCR_TEIE;
    stream->base.handler = interruptHandler;
    stream->callback = 0;
    stream->state = STATE_IDLE;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  DmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void streamConfigure(void *object, const void *settingsBase)
{
  const struct DmaSettings * const settings = settingsBase;
  struct DmaCircular * const stream = object;

  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  uint32_t config = stream->base.config
      & ~(CCR_PINC | CCR_MINC | CCR_PSIZE_MASK | CCR_MSIZE_MASK);

  if (config & CCR_DIR)
  {
    /* Direction is from memory to peripheral */
    config |= CCR_MSIZE(settings->source.width)
        | CCR_PSIZE(settings->destination.width);

    if (settings->source.increment)
      config |= CCR_MINC;
    if (settings->destination.increment)
      config |= CCR_PINC;
  }
  else
  {
    /* Direction is from peripheral to memory */
    config |= CCR_PSIZE(settings->source.width)
        | CCR_MSIZE(settings->destination.width);

    if (settings->source.increment)
      config |= CCR_PINC;
    if (settings->destination.increment)
      config |= CCR_MINC;
  }

  config |= CCR_CIRC;

  stream->base.config = config;
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DmaCircular * const stream = object;

  stream->callback = callback;
  stream->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result streamEnable(void *object)
{
  struct DmaCircular * const stream = object;
  const uint8_t number = stream->base.number;

  assert(stream->state == STATE_READY);

  if (dmaSetInstance(number, object))
  {
    STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

    reg->CMAR = stream->memoryAddress;
    reg->CPAR = stream->periphAddress;
    reg->CNDTR = stream->transfers;

    /* Start the transfer */
    stream->state = STATE_BUSY;
    reg->CCR = stream->base.config | CCR_EN;

    return E_OK;
  }
  else
  {
    stream->state = STATE_ERROR;
    return E_BUSY;
  }
}
/*----------------------------------------------------------------------------*/
static void streamDisable(void *object)
{
  struct DmaCircular * const stream = object;
  STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

  /*
   * Incorrect sequence of calls: stream should not be disabled when
   * it is not initialized and started.
   */
  assert(stream->state != STATE_IDLE && stream->state != STATE_READY);

  if (stream->state == STATE_BUSY)
  {
    reg->CCR &= ~CCR_EN;
    stream->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamResidue(const void *object, size_t *count)
{
  const struct DmaCircular * const stream = object;

  if (stream->state != STATE_IDLE && stream->state != STATE_READY)
  {
    const STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

    *count = reg->CNDTR;
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result streamStatus(const void *object)
{
  const struct DmaCircular * const stream = object;

  switch ((enum State)stream->state)
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
static void streamAppend(void *object, void *destination, const void *source,
    size_t size)
{
  struct DmaCircular * const stream = object;

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= DMA_MAX_TRANSFER);
  assert(size % 2 == 0);
  assert(stream->state != STATE_BUSY && stream->state != STATE_READY);

  if (stream->base.config & CCR_DIR)
  {
    /* Direction is from memory to peripheral */
    stream->memoryAddress = (uintptr_t)source;
    stream->periphAddress = (uintptr_t)destination;
  }
  else
  {
    /* Direction is from peripheral to memory */
    stream->memoryAddress = (uintptr_t)destination;
    stream->periphAddress = (uintptr_t)source;
  }

  stream->transfers = (uint16_t)size;
  stream->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void streamClear(void *object)
{
  ((struct DmaCircular *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t streamQueued(const void *object)
{
  const struct DmaCircular * const stream = object;

  if (stream->state != STATE_IDLE && stream->state != STATE_READY)
  {
    const STM_DMA_CHANNEL_Type * const reg = stream->base.reg;
    return reg->CNDTR <= (stream->transfers >> 1);
  }
  else
    return 0;
}
