/*
 * dma_circular.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_circular.h>
#include <halm/platform/stm32/dma_defs.h>
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
    STM_DMA_STREAM_Type * const reg = stream->base.reg;

    reg->CR = 0;
    dmaResetInstance(stream->base.number);

    stream->state = STATE_ERROR;
  }

  if (stream->callback != NULL)
    stream->callback(stream->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct DmaCircularConfig * const config = configBase;
  assert(config != NULL);

  const struct DmaBaseConfig baseConfig = {
      .event = config->event,
      .priority = config->priority,
      .type = config->type,
      .stream = config->stream
  };
  struct DmaCircular * const stream = object;

  /* Call base class constructor */
  const enum Result res = DmaBase->init(stream, &baseConfig);

  if (res == E_OK)
  {
    stream->base.config |= SCR_TCIE | SCR_TEIE;
    if (!config->silent)
      stream->base.config |= SCR_HTIE;

    stream->base.handler = interruptHandler;
    stream->callback = NULL;
    stream->fifo = 0;
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

  assert(settings->source.burst <= DMA_BURST_16);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst <= DMA_BURST_16);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  /* Burst size must not exceed FIFO size */
  assert(burstToSize(settings->source.width,
      settings->source.burst) <= 16);
  assert(burstToSize(settings->destination.width,
      settings->destination.burst) <= 16);

  /* Burst transfer is not allowed in the fixed address mode */
  assert(settings->source.increment
      || settings->source.burst == DMA_BURST_1);
  assert(settings->destination.increment
      || settings->destination.burst == DMA_BURST_1);

  uint32_t config = stream->base.config & ~(
      SCR_PINC | SCR_PSIZE_MASK | SCR_PBURST_MASK
      | SCR_MINC | SCR_MSIZE_MASK | SCR_MBURST_MASK
  );

  if (SCR_DIR_VALUE(config) == DMA_TYPE_M2P)
  {
    /* Direction is from memory to peripheral */
    config |=
        SCR_PSIZE(settings->destination.width)
        | SCR_MSIZE(settings->source.width)
        | SCR_PBURST(settings->destination.burst)
        | SCR_MBURST(settings->source.burst);

    if (settings->source.increment)
      config |= SCR_MINC;
    if (settings->destination.increment)
      config |= SCR_PINC;
  }
  else
  {
    /* Direction is from peripheral to memory */
    config |=
        SCR_PSIZE(settings->source.width)
        | SCR_MSIZE(settings->destination.width)
        | SCR_PBURST(settings->source.burst)
        | SCR_MBURST(settings->destination.burst);

    if (settings->source.increment)
      config |= SCR_PINC;
    if (settings->destination.increment)
      config |= SCR_MINC;
  }

  if (settings->source.width != settings->destination.width
      || settings->source.burst != DMA_BURST_1
      || settings->destination.burst != DMA_BURST_1)
  {
    /* Disable direct mode */
    stream->fifo = SFCR_DMDIS;

    if (SCR_DIR_VALUE(config) == DMA_TYPE_M2P)
    {
      stream->fifo |= burstToThreshold(settings->source.width,
          settings->source.burst);
    }
    else
    {
      stream->fifo |= burstToThreshold(settings->destination.width,
          settings->destination.burst);
    }
  }
  else
  {
    /* Enable direct mode */
    stream->fifo = 0;
  }

  config |= SCR_CIRC;
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

  assert(stream->state == STATE_READY);

  if (dmaSetInstance(stream->base.number, object))
  {
    STM_DMA_STREAM_Type * const reg = stream->base.reg;

    stream->state = STATE_BUSY;
    reg->FCR = (uint32_t)stream->fifo;
    reg->NDTR = stream->transfers;
    reg->M0AR = stream->memoryAddress;
    reg->PAR = stream->periphAddress;

    /* Start the transfer */
    __dsb();
    reg->CR = stream->base.config | SCR_EN;

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
  STM_DMA_STREAM_Type * const reg = stream->base.reg;

  if (stream->state == STATE_BUSY)
  {
    reg->CR &= ~(SCR_EN | SCR_IE_MASK);
    dmaResetInstance(stream->base.number);

    stream->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamResidue(const void *object, size_t *count)
{
  const struct DmaCircular * const stream = object;

  if (stream->state != STATE_IDLE && stream->state != STATE_READY)
  {
    const STM_DMA_STREAM_Type * const reg = stream->base.reg;

    *count = reg->NDTR;
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

  assert(destination != NULL && source != NULL);
  assert(size > 0 && size <= DMA_MAX_TRANSFER);
  assert(!(stream->base.config & SCR_HTIE) || (size % 2 == 0));
  assert(stream->state != STATE_BUSY && stream->state != STATE_READY);

  if (SCR_DIR_VALUE(stream->base.config) == DMA_TYPE_M2P)
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
    const STM_DMA_STREAM_Type * const reg = stream->base.reg;
    const bool lower = reg->NDTR <= (stream->transfers >> 1);

    return lower ? 1 : 2;
  }
  else
    return 0;
}
