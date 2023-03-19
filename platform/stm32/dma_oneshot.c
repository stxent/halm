/*
 * dma_oneshot.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_defs.h>
#include <halm/platform/stm32/dma_oneshot.h>
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
const struct DmaClass * const DmaOneShot = &(const struct DmaClass){
    .size = sizeof(struct DmaOneShot),
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
  struct DmaOneShot * const stream = object;
  STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

  /* Disable the DMA stream */
  reg->CCR = 0;

  dmaResetInstance(stream->base.number);
  stream->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (stream->callback)
    stream->callback(stream->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct DmaOneShotConfig * const config = configBase;
  assert(config);

  const struct DmaBaseConfig baseConfig = {
      .type = config->type,
      .stream = config->stream,
      .priority = config->priority
  };
  struct DmaOneShot * const stream = object;

  /* Call base class constructor */
  const enum Result res = DmaBase->init(stream, &baseConfig);

  if (res == E_OK)
  {
    stream->base.config |= CCR_TCIE | CCR_TEIE;
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
  struct DmaOneShot * const stream = object;

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

  stream->base.config = config;
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DmaOneShot * const stream = object;

  stream->callback = callback;
  stream->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result streamEnable(void *object)
{
  struct DmaOneShot * const stream = object;
  const uint8_t number = stream->base.number;

  assert(stream->state == STATE_READY);

  if (dmaSetInstance(number, object))
  {
    STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

    stream->state = STATE_BUSY;
    reg->CMAR = stream->memoryAddress;
    reg->CPAR = stream->periphAddress;
    reg->CNDTR = stream->transfers;

    /* Start the transfer */
    __dsb();
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
  struct DmaOneShot * const stream = object;
  STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

  if (stream->state == STATE_BUSY)
  {
    reg->CCR &= ~CCR_EN;
    dmaResetInstance(stream->base.number);

    stream->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamResidue(const void *object, size_t *count)
{
  const struct DmaOneShot * const stream = object;

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
  const struct DmaOneShot * const stream = object;

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
  struct DmaOneShot * const stream = object;

  assert(destination != 0 && source != 0);
  assert(size > 0 && size <= DMA_MAX_TRANSFER);
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
  ((struct DmaOneShot *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t streamQueued(const void *object)
{
  const struct DmaOneShot * const stream = object;
  return stream->state != STATE_IDLE && stream->state != STATE_READY ? 1 : 0;
}
