/*
 * dma_list.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/dma_defs.h>
#include <halm/platform/stm32/dma_list.h>
#include <xcore/asm.h>
#include <assert.h>
#include <stdlib.h>
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
const struct DmaClass * const DmaList = &(const struct DmaClass){
    .size = sizeof(struct DmaList),
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
  struct DmaList * const stream = object;
  STM_DMA_STREAM_Type * const reg = stream->base.reg;
  bool event = false;

  --stream->queued;

  switch (res)
  {
    case E_OK:
    {
      if (stream->queued >= 1)
      {
        size_t index = stream->index;

        /* Calculate index of the first chunk */
        if (index >= stream->queued)
          index -= stream->queued;
        else
          index += stream->capacity - stream->queued;

        if (reg->CR & SCR_CT)
          reg->M0AR = stream->list[index].memoryAddress;
        else
          reg->M1AR = stream->list[index].memoryAddress;

        if (stream->queued == 1)
        {
          /* Queue drained, last buffer will be dropped */
          reg->CR &= ~SCR_EN;
        }
      }
      else
        stream->state = STATE_IDLE;

      event = true;
      break;
    }

    default:
      stream->state = STATE_ERROR;
      break;
  }

  if (stream->state != STATE_BUSY)
  {
    reg->CR = 0;
    dmaResetInstance(stream->base.number);

    event = true;
  }

  if (event && stream->callback != NULL)
    stream->callback(stream->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct DmaListConfig * const config = configBase;
  assert(config != NULL);

  const struct DmaBaseConfig baseConfig = {
      .event = config->event,
      .priority = config->priority,
      .type = config->type,
      .stream = config->stream
  };
  struct DmaList * const stream = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = DmaBase->init(stream, &baseConfig)) != E_OK)
    return res;

  stream->list = malloc(sizeof(struct DmaListEntry) * config->number);
  if (stream->list == NULL)
    res = E_MEMORY;

  if (res == E_OK)
  {
    stream->base.config |= SCR_TCIE | SCR_TEIE;
    stream->base.handler = interruptHandler;
  
    stream->callback = NULL;
    stream->capacity = config->number;
    stream->periphAddress = 0;
    stream->transfers = 0;
    stream->index = 0;
    stream->queued = 0;
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
  struct DmaList * const stream = object;

  assert(settings->source.burst <= DMA_BURST_16);
  assert(settings->source.width <= DMA_WIDTH_WORD);
  assert(settings->destination.burst <= DMA_BURST_16);
  assert(settings->destination.width <= DMA_WIDTH_WORD);

  /* Burst size must not exceed FIFO size */
  assert(burstToSize(settings->source.width,
      settings->source.burst) <= FIFO_MAX_SIZE);
  assert(burstToSize(settings->destination.width,
      settings->destination.burst) <= FIFO_MAX_SIZE);

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

  stream->base.config = config;
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DmaList * const stream = object;

  stream->callback = callback;
  stream->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result streamEnable(void *object)
{
  struct DmaList * const stream = object;

  assert(stream->state == STATE_READY);

  if (dmaSetInstance(stream->base.number, object))
  {
    STM_DMA_STREAM_Type * const reg = stream->base.reg;
    uint32_t config = stream->base.config | SCR_EN;

    stream->state = STATE_BUSY;
    reg->FCR = (uint32_t)stream->fifo;
    reg->NDTR = stream->transfers;
    reg->M0AR = stream->list[0].memoryAddress;
    reg->PAR = stream->periphAddress;

    if (stream->queued > 1)
    {
      config |= SCR_DBM;
      reg->M1AR = stream->list[1].memoryAddress;
    }
    else
      reg->M1AR = 0;

    /* Start the transfer */
    __dmb();
    reg->CR = config;

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
  struct DmaList * const stream = object;
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
  const struct DmaList * const stream = object;

  /* Residue is available when the stream was initialized and enabled */
  if (stream->state != STATE_IDLE && stream->state != STATE_READY)
  {
    const STM_DMA_STREAM_Type * const reg = stream->base.reg;
    const unsigned int target = (reg->CR & SCR_CT) != 0;
    size_t index = stream->index + stream->capacity - stream->queued;

    if (index >= stream->capacity)
      index -= stream->capacity;

    const uint32_t config = stream->base.config;
    const uint32_t width = SCR_DIR_VALUE(config) == DMA_TYPE_M2P ?
        (1 << SCR_PSIZE_VALUE(config)) : (1 << SCR_MSIZE_VALUE(config));

    if (((reg->CR & SCR_CT) != 0) == target)
    {
      /* Linked list item is not changed, transfer count is correct */
      *count = (size_t)(reg->NDTR * width);
      return E_OK;
    }
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result streamStatus(const void *object)
{
  const struct DmaList * const stream = object;

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
  struct DmaList * const stream = object;
  const uint32_t config = stream->base.config;

  assert(destination != NULL && source != NULL);
  assert(!(size % (1 << SCR_PSIZE_VALUE(config))));
  assert(!(size % (1 << SCR_MSIZE_VALUE(config))));
  assert(stream->queued < stream->capacity);

  if (stream->state == STATE_DONE || stream->state == STATE_ERROR)
  {
    stream->index = 0;
    stream->queued = 0;
  }

  struct DmaListEntry * const entry = stream->list + stream->index;
  uintptr_t periphAddress;
  uint32_t transfers;

  if (++stream->index >= stream->capacity)
    stream->index = 0;

  if (SCR_DIR_VALUE(config) == DMA_TYPE_M2P)
  {
    /* Direction is from memory to peripheral */
    entry->memoryAddress = (uintptr_t)source;
    periphAddress = (uintptr_t)destination;
    transfers = size >> SCR_PSIZE_VALUE(config);
  }
  else
  {
    /* Direction is from peripheral to memory */
    entry->memoryAddress = (uintptr_t)destination;
    periphAddress = (uintptr_t)source;
    transfers = size >> SCR_MSIZE_VALUE(config);
  }

  assert(!(entry->memoryAddress % (1 << SCR_MSIZE_VALUE(config))));
  assert(!(periphAddress % (1 << SCR_PSIZE_VALUE(config))));
  assert(transfers && transfers <= DMA_MAX_TRANSFER);
  assert(!stream->transfers || transfers == stream->transfers);
  assert(!stream->transfers || periphAddress == stream->periphAddress);

  ++stream->queued;

  if (stream->state != STATE_BUSY)
  {
    stream->periphAddress = periphAddress;
    stream->transfers = (uint16_t)transfers;
    stream->state = STATE_READY;
  }
}
/*----------------------------------------------------------------------------*/
static void streamClear(void *object)
{
  struct DmaList * const stream = object;

  stream->index = 0;
  stream->queued = 0;
  stream->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t streamQueued(const void *object)
{
  const struct DmaList * const stream = object;

  if (stream->state != STATE_IDLE)
    return stream->queued;
  return 0;
}
