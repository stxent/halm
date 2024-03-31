/*
 * edma_circular.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/edma_circular.h>
#include <halm/platform/imxrt/edma_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/asm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_READY,
  STATE_BUSY,
  STATE_DONE,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *, enum Result);
static inline int32_t widthToBytes(enum EdmaWidth);
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
const struct DmaClass * const EdmaCircular = &(const struct DmaClass){
    .size = sizeof(struct EdmaCircular),
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
  struct EdmaCircular * const channel = object;
  IMX_EDMA_Type * const reg = channel->base.reg;
  const uint8_t number = channel->base.number;

  if (res == E_OK)
  {
    /* Clear pending interrupt flag */
    reg->CINT = CINT_CINT(number);
  }
  else
  {
    /* Disable further hardware requests */
    reg->CERQ = CERQ_CERQ(number);
    /* Clear pending interrupt flag */
    reg->CERR = CERR_CERR(number);

    /* Release the channel */
    edmaUnbindInstance(&channel->base);

    channel->state = STATE_ERROR;
  }

  if (channel->callback != NULL)
    channel->callback(channel->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static inline int32_t widthToBytes(enum EdmaWidth width)
{
  /* Direct conversion from enumeration to DSIZE and SSIZE values */
  return 1 << width;
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct EdmaCircularConfig * const config = configBase;
  assert(config != NULL);

  const struct EdmaBaseConfig baseConfig = {
      .event = config->event,
      .priority = config->priority,
      .channel = config->channel
  };
  struct EdmaCircular * const channel = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = EdmaBase->init(channel, &baseConfig)) != E_OK)
    return res;

  channel->base.handler = interruptHandler;

  channel->callback = NULL;
  channel->silent = config->silent;
  channel->state = STATE_IDLE;

  channel->destination = 0;
  channel->source = 0;
  channel->count = 0;
  channel->attributes = 0;
  channel->burst = 0;
  channel->dstOffset = 0;
  channel->srcOffset = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDeinit(void *object)
{
  if (EdmaBase->deinit != NULL)
    EdmaBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static void channelConfigure(void *object, const void *settingsBase)
{
  const struct EdmaSettings * const settings = settingsBase;
  struct EdmaCircular * const channel = object;

  assert(settings->source.width <= DMA_WIDTH_DOUBLEWORD);
  assert(settings->destination.width <= DMA_WIDTH_DOUBLEWORD);

  const uint32_t dstWidth = widthToBytes(settings->destination.width);
  const uint32_t srcWidth = widthToBytes(settings->source.width);
  const uint32_t minorByteCount = settings->burst * MAX(dstWidth, srcWidth);

  assert(minorByteCount <= TCD_NBYTES_MLOFFNO_NBYTES_MAX);
  assert(settings->destination.offset >= TCD_OFF_MIN
      && settings->destination.offset <= TCD_OFF_MAX);
  assert(settings->source.offset >= TCD_OFF_MIN
      && settings->source.offset <= TCD_OFF_MAX);

  channel->attributes = TCD_ATTR_DSIZE(settings->destination.width)
      | TCD_ATTR_SSIZE(settings->source.width);
  channel->burst = minorByteCount;
  channel->dstOffset = settings->destination.offset;
  channel->srcOffset = settings->source.offset;
}
/*----------------------------------------------------------------------------*/
static void channelSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct EdmaCircular * const channel = object;

  assert(channel->state != STATE_BUSY);

  channel->callback = callback;
  channel->callbackArgument = argument;
}
/*----------------------------------------------------------------------------*/
static enum Result channelEnable(void *object)
{
  struct EdmaCircular * const channel = object;
  IMX_EDMA_Type * const reg = channel->base.reg;
  IMX_EDMA_TCD_Type * const tcd = &reg->TCD[channel->base.number];

  assert(channel->state == STATE_READY || channel->state == STATE_DONE);

  if (!edmaBindInstance(&channel->base))
  {
    channel->state = STATE_ERROR;
    return E_BUSY;
  }

  edmaSetMux(&channel->base);
  channel->state = STATE_BUSY;

  const uint32_t majorLoopCount = channel->count / channel->burst;

  tcd->BITER_ELINKNO = TCD_BITER_ELINKNO_BITER(majorLoopCount);
  tcd->CITER_ELINKNO = TCD_CITER_ELINKNO_CITER(majorLoopCount);
  tcd->NBYTES_MLOFFNO = TCD_NBYTES_MLOFFNO_NBYTES(channel->burst);
  tcd->ATTR = channel->attributes;
  tcd->SADDR = channel->source;
  tcd->SOFF = channel->srcOffset;
  tcd->SLAST = channel->srcOffset == 0 ? 0
      : ((channel->srcOffset > 0) ? -channel->count : channel->count);
  tcd->DADDR = channel->destination;
  tcd->DOFF = channel->dstOffset;
  tcd->DLAST_SGA = channel->dstOffset == 0 ? 0
      : ((channel->dstOffset > 0) ? -channel->count : channel->count);
  tcd->CSR = TCD_CSR_INTMAJOR | TCD_CSR_INTHALF;

  edmaStartTransfer(&channel->base);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void channelDisable(void *object)
{
  struct EdmaCircular * const channel = object;

  if (channel->state == STATE_BUSY)
  {
    IMX_EDMA_Type * const reg = channel->base.reg;
    const uint8_t number = channel->base.number;

    edmaResetMux(&channel->base);
    while (reg->HRS & (1UL << number));
    reg->CERQ = CERQ_CERQ(number);

    edmaUnbindInstance(&channel->base);
    channel->state = STATE_DONE;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelResidue(const void *object, size_t *count)
{
  const struct EdmaCircular * const channel = object;

  /* Residue is available when the channel was initialized and enabled */
  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const IMX_EDMA_Type * const reg = channel->base.reg;
    const IMX_EDMA_TCD_Type * const tcd = &reg->TCD[channel->base.number];
    const uint32_t citer = TCD_CITER_ELINKNO_CITER_VALUE(tcd->CITER_ELINKNO);

    *count = (size_t)(citer * channel->burst);
    return E_OK;
  }

  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result channelStatus(const void *object)
{
  const struct EdmaCircular * const channel = object;

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
  struct EdmaCircular * const channel = object;

  assert(destination != NULL && source != NULL);
  assert(!((uintptr_t)destination
      % (1 << TCD_ATTR_DSIZE(channel->attributes))));
  assert(!((uintptr_t)source
      % (1 << TCD_ATTR_SSIZE(channel->attributes))));
  assert(!(size % channel->burst));
  assert(size / channel->burst <= TCD_BITER_ELINKNO_BITER_MAX);
  assert(channel->state != STATE_BUSY);

  channel->destination = (uintptr_t)destination;
  channel->source = (uintptr_t)source;
  channel->count = (uint32_t)size;
  channel->state = STATE_READY;
}
/*----------------------------------------------------------------------------*/
static void channelClear(void *object)
{
  ((struct EdmaCircular *)object)->state = STATE_IDLE;
}
/*----------------------------------------------------------------------------*/
static size_t channelQueued(const void *object)
{
  const struct EdmaCircular * const channel = object;

  if (channel->state != STATE_IDLE && channel->state != STATE_READY)
  {
    const IMX_EDMA_Type * const reg = channel->base.reg;
    const IMX_EDMA_TCD_Type * const tcd = &reg->TCD[channel->base.number];
    const uint32_t biter = TCD_BITER_ELINKNO_BITER_VALUE(tcd->BITER_ELINKNO);
    const uint32_t citer = TCD_CITER_ELINKNO_CITER_VALUE(tcd->CITER_ELINKNO);

    return citer <= (biter >> 1) ? 1 : 2;
  }
  else
    return 0;
}
