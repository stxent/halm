/*
 * dac_dma.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/dac_dma.h>
#include <halm/platform/lpc/gen_1/dac_defs.h>
#include <halm/platform/lpc/gpdma_list.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct DacDmaStreamConfig
{
  /** Mandatory: pointer to a parent object. */
  struct DacDma *parent;
  /** Mandatory: queue size. */
  size_t size;
};

struct DacDmaStream
{
  struct Stream base;

  /* Parent interface */
  struct DacDma *parent;
  /* Queued requests */
  PointerQueue requests;
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object);
static bool dmaSetup(struct DacDma *, const struct DacDmaConfig *);
static uint32_t getConversionRate(const struct DacDma *);
static void setConversionRate(struct DacDma *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *, const void *);
static enum Result dacGetParam(void *, int, void *);
static enum Result dacSetParam(void *, int, const void *);

static enum Result dacStreamInit(void *, const void *);
static void dacStreamClear(void *);
static enum Result dacStreamEnqueue(void *, struct StreamRequest *);

#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *);
static void dacStreamDeinit(void *);
#else
#define dacDeinit deletedDestructorTrap
#define dacStreamDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const DacDma = &(const struct InterfaceClass){
    .size = sizeof(struct DacDma),
    .init = dacInit,
    .deinit = dacDeinit,

    .setCallback = 0,
    .getParam = dacGetParam,
    .setParam = dacSetParam,
    .read = 0,
    .write = 0
};

const struct StreamClass * const DacDmaStream = &(const struct StreamClass){
    .size = sizeof(struct DacDmaStream),
    .init = dacStreamInit,
    .deinit = dacStreamDeinit,

    .clear = dacStreamClear,
    .enqueue = dacStreamEnqueue
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct DacDma * const interface = object;
  const enum Result transferStatus = dmaStatus(interface->dma);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;
  bool event = false;

  if (transferStatus != E_BUSY)
  {
    LPC_DAC_Type * const reg = interface->base.reg;

    /* Scatter-gather transfer finished, disable further requests */
    reg->CTRL &= ~(CTRL_INT_DMA_REQ | CTRL_CNT_ENA);

    if (transferStatus == E_ERROR)
      requestStatus = STREAM_REQUEST_FAILED;

    event = true;
  }
  else if ((dmaQueued(interface->dma) & 1) == 0)
  {
    event = true;
  }

  if (event)
  {
    struct StreamRequest * const request =
        pointerQueueFront(&interface->stream->requests);
    pointerQueuePopFront(&interface->stream->requests);

    request->callback(request->argument, request, requestStatus);
  }
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct DacDma *interface,
    const struct DacDmaConfig *config)
{
  static const struct GpDmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_HALFWORD,
          .increment = true
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      }
  };
  const struct GpDmaListConfig dmaConfig = {
      .number = config->size << 1,
      .event = GPDMA_DAC,
      .type = GPDMA_TYPE_M2P,
      .channel = config->dma
  };

  interface->dma = init(GpDmaList, &dmaConfig);

  if (interface->dma)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static uint32_t getConversionRate(const struct DacDma *interface)
{
  const LPC_DAC_Type * const reg = interface->base.reg;
  const uint32_t clock = dacGetClock(&interface->base);

  return clock / (reg->CNTVAL + 1);
}
/*----------------------------------------------------------------------------*/
static void setConversionRate(struct DacDma *interface, uint32_t rate)
{
  LPC_DAC_Type * const reg = interface->base.reg;
  const uint32_t clock = dacGetClock(&interface->base);

  reg->CNTVAL = (clock + (rate - 1)) / rate - 1;
}
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *object, const void *configBase)
{
  const struct DacDmaConfig * const config = configBase;
  assert(config);
  assert(config->rate);

  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  const struct DacDmaStreamConfig streamConfig = {
      .parent = object,
      .size = config->size
  };

  struct DacDma * const interface = object;

  /* Call base class constructor */
  const enum Result res = DacBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->stream = init(DacDmaStream, &streamConfig);
  if (!interface->stream)
    return E_ERROR;

  if (!dmaSetup(interface, config))
    return E_ERROR;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  setConversionRate(interface, config->rate);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result dacGetParam(void *object, int parameter, void *data)
{
  struct DacDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_TX_AVAILABLE:
      *(size_t *)data = pointerQueueCapacity(&interface->stream->requests)
          - pointerQueueSize(&interface->stream->requests);
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = pointerQueueSize(&interface->stream->requests);
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = getConversionRate(interface);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dacSetParam(void *object, int parameter, const void *data)
{
  struct DacDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      setConversionRate(interface, *(const uint32_t *)data);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dacStreamInit(void *object, const void *configBase)
{
  const struct DacDmaStreamConfig * const config = configBase;
  struct DacDmaStream * const stream = object;

  if (pointerQueueInit(&stream->requests, config->size))
  {
    stream->parent = config->parent;
    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void dacStreamClear(void *object)
{
  struct DacDmaStream * const stream = object;

  while (!pointerQueueEmpty(&stream->requests))
  {
    struct StreamRequest * const request = pointerQueueFront(&stream->requests);
    pointerQueuePopFront(&stream->requests);

    request->callback(request->argument, request, STREAM_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dacStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct DacDmaStream * const stream = object;
  struct DacDma * const interface = stream->parent;

  assert(request && request->callback);
  /* Ensure the buffer has enough space and is aligned with the sample size */
  assert(request->capacity / sizeof(uint16_t) >= 2);
  assert(request->capacity % sizeof(uint16_t) == 0);
  /* Output buffer should be aligned with the sample size */
  assert((uintptr_t)request->buffer % sizeof(uint16_t) == 0);

  /* Prepare linked list of DMA descriptors */
  const size_t samples = request->capacity / sizeof(uint16_t);
  const size_t parts[] = {samples >> 1, samples - (samples >> 1)};

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    LPC_DAC_Type * const reg = interface->base.reg;
    const uint16_t * const src = request->buffer;

    /* When a previous transfer is ongoing it will be continued */
    dmaAppend(interface->dma, (void *)&reg->CR, src, parts[0]);
    dmaAppend(interface->dma, (void *)&reg->CR, src + parts[0], parts[1]);

    if (dmaStatus(interface->dma) != E_BUSY)
    {
      if (dmaEnable(interface->dma) == E_OK)
      {
        /* Enable counter to generate memory access requests */
        reg->CTRL |= CTRL_CNT_ENA;
      }
      else
        res = E_INTERFACE;
    }
  }
  else
    res = E_FULL;

  if (res == E_OK)
    pointerQueuePushBack(&stream->requests, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *object)
{
  struct DacDma * const interface = object;

  deinit(interface->stream);
  deinit(interface->dma);
  DacBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacStreamDeinit(void *object)
{
  struct DacDmaStream * const stream = object;
  pointerQueueDeinit(&stream->requests);
}
#endif
/*----------------------------------------------------------------------------*/
struct Stream *dacDmaGetOutput(struct DacDma *interface)
{
  return (struct Stream *)interface->stream;
}
