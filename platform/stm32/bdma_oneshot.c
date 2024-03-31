/*
 * bdma_oneshot.c
 * Copyright (C) 2018, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/bdma_defs.h>
#include <halm/platform/stm32/bdma_oneshot.h>
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
const struct DmaClass * const BdmaOneShot = &(const struct DmaClass){
    .size = sizeof(struct BdmaOneShot),
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
  struct BdmaOneShot * const stream = object;
  STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

  /* Disable the DMA stream */
  reg->CCR = 0;

  bdmaResetInstance(stream->base.number);
  stream->state = res == E_OK ? STATE_DONE : STATE_ERROR;

  if (stream->callback != NULL)
    stream->callback(stream->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct BdmaOneShotConfig * const config = configBase;
  assert(config != NULL);

  const struct BdmaBaseConfig baseConfig = {
      .event = config->event,
      .priority = config->priority,
      .type = config->type,
      .stream = config->stream
  };
  struct BdmaOneShot * const stream = object;

  /* Call base class constructor */
  const enum Result res = BdmaBase->init(stream, &baseConfig);

  if (res == E_OK)
  {
    stream->base.config |= CCR_TCIE | CCR_TEIE;
    stream->base.handler = interruptHandler;
    stream->callback = NULL;
    stream->state = STATE_IDLE;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  BdmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void streamConfigure(void *object, const void *settingsBase)
{
  const struct DmaSettings * const settings = settingsBase;
  struct BdmaOneShot * const stream = object;

  /* Burst transfers are not supported */
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
  struct BdmaOneShot * const stream = object;

  stream->callback = callback;
  stream->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result streamEnable(void *object)
{
  struct BdmaOneShot * const stream = object;

  assert(stream->state == STATE_READY);

  if (bdmaSetInstance(stream->base.number, object))
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
  struct BdmaOneShot * const stream = object;
  STM_DMA_CHANNEL_Type * const reg = stream->base.reg;

  if (stream->state == STATE_BUSY)
  {
    reg->CCR &= ~(CCR_EN | CCR_IE_MASK);
    bdmaResetInstance(stream->base.number);

    stream->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamResidue(const void *object, size_t *count)
{
  const struct BdmaOneShot * const stream = object;

  if (stream->state != STATE_IDLE && stream->state != STATE_READY)
  {
    const STM_DMA_CHANNEL_Type * const reg = stream->base.reg;
    const uint32_t config = stream->base.config;
    const uint32_t width = (config & CCR_DIR) ?
        (1 << CCR_PSIZE_VALUE(config)) : (1 << CCR_MSIZE_VALUE(config));

    *count = (size_t)(reg->CNDTR * width);
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result streamStatus(const void *object)
{
  const struct BdmaOneShot * const stream = object;

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
  struct BdmaOneShot * const stream = object;
  const uint32_t config = stream->base.config;
  uintptr_t memoryAddress;
  uintptr_t periphAddress;
  uint32_t transfers;

  assert(destination != NULL && source != NULL);
  assert(!(size % (1 << CCR_PSIZE_VALUE(config))));
  assert(!(size % (1 << CCR_MSIZE_VALUE(config))));
  assert(stream->state != STATE_BUSY && stream->state != STATE_READY);

  if (config & CCR_DIR)
  {
    /* Direction is from memory to peripheral */
    memoryAddress = (uintptr_t)source;
    periphAddress = (uintptr_t)destination;
    transfers = size >> CCR_PSIZE_VALUE(config);
  }
  else
  {
    /* Direction is from peripheral to memory */
    memoryAddress = (uintptr_t)destination;
    periphAddress = (uintptr_t)source;
    transfers = size >> CCR_MSIZE_VALUE(config);
  }

  assert(!(periphAddress % (1 << CCR_PSIZE_VALUE(config))));
  assert(!(memoryAddress % (1 << CCR_MSIZE_VALUE(config))));
  assert(transfers > 0 && transfers <= DMA_MAX_TRANSFER);

  stream->memoryAddress = memoryAddress;
  stream->periphAddress = periphAddress;
  stream->transfers = (uint16_t)transfers;
  stream->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void streamClear(void *object)
{
  ((struct BdmaOneShot *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t streamQueued(const void *object)
{
  const struct BdmaOneShot * const stream = object;
  return stream->state != STATE_IDLE && stream->state != STATE_READY ? 1 : 0;
}
