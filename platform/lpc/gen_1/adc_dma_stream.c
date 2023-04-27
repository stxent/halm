/*
 * adc_dma_stream.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/adc_dma_stream.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/gpdma_circular.h>
#include <halm/platform/lpc/gpdma_list.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct AdcDmaStreamHandlerConfig
{
  /** Mandatory: pointer to a parent object. */
  struct AdcDmaStream *parent;
  /** Mandatory: queue size. */
  size_t size;
};

struct AdcDmaStreamHandler
{
  struct Stream base;

  /* Parent interface */
  struct AdcDmaStream *parent;
  /* Queued requests */
  PointerQueue requests;
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static void resetInnerBuffers(struct AdcDmaStream *);
static bool setupInnerChannel(struct AdcDmaStream *,
    const struct AdcDmaStreamConfig *);
static bool setupOuterChannel(struct AdcDmaStream *,
    const struct AdcDmaStreamConfig *);
static size_t setupPins(struct AdcDmaStream *, const PinNumber *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);

static enum Result adcHandlerInit(void *, const void *);
static void adcHandlerClear(void *);
static enum Result adcHandlerEnqueue(void *, struct StreamRequest *);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
static void adcHandlerDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#define adcHandlerDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcDmaStream =
    &(const struct InterfaceClass){
    .size = sizeof(struct AdcDmaStream),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = NULL,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = NULL,
    .write = NULL
};

const struct StreamClass * const AdcDmaStreamHandler =
    &(const struct StreamClass){
    .size = sizeof(struct AdcDmaStreamHandler),
    .init = adcHandlerInit,
    .deinit = adcHandlerDeinit,

    .clear = adcHandlerClear,
    .enqueue = adcHandlerEnqueue
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct AdcDmaStream * const interface = object;
  const enum Result transferStatus = dmaStatus(interface->outer);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;
  bool event = false;

  /* Scatter-gather transfer finished */
  if (transferStatus != E_BUSY)
  {
    LPC_ADC_Type * const reg = interface->base.reg;

    /* Stop automatic conversion and disable further requests */
    reg->CR = 0;
    reg->INTEN = 0;

    dmaDisable(interface->inner);

    if (transferStatus == E_ERROR)
      requestStatus = STREAM_REQUEST_FAILED;

    event = true;
  }
  else if ((dmaQueued(interface->outer) & 1) == 0)
  {
    /*
     * Each block consists of two buffers. Call user function
     * at the end of the odd block or at the end of the list.
     */
    event = true;
  }

  if (event)
  {
    struct StreamRequest * const request =
        pointerQueueFront(&interface->stream->requests);
    pointerQueuePopFront(&interface->stream->requests);

    if (requestStatus == STREAM_REQUEST_COMPLETED)
      request->length = request->capacity;
    request->callback(request->argument, request, requestStatus);
  }
}
/*----------------------------------------------------------------------------*/
static void resetInnerBuffers(struct AdcDmaStream *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;

  for (size_t index = 0; index < interface->count; ++index)
  {
    dmaAppend(interface->inner, &interface->buffer,
        (const void *)&reg->DR[interface->pins[index].channel], 1);
  }
}
/*----------------------------------------------------------------------------*/
static bool setupInnerChannel(struct AdcDmaStream *interface,
    const struct AdcDmaStreamConfig *config)
{
  static const struct GpDmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      }
  };

  /* Silent mode is ignored because callback is not configured */
  const struct GpDmaCircularConfig dmaConfig = {
      .number = interface->count,
      .event = GPDMA_ADC0 + config->channel,
      .type = GPDMA_TYPE_P2M,
      .channel = config->converter.dma,
      .silent = false
  };

  interface->inner = init(GpDmaCircular, &dmaConfig);

  if (interface->inner != NULL)
  {
    dmaConfigure(interface->inner, &dmaSettings);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static bool setupOuterChannel(struct AdcDmaStream *interface,
    const struct AdcDmaStreamConfig *config)
{
  static const struct GpDmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_WORD,
          .increment = true
      }
  };
  const struct GpDmaListConfig dmaConfig = {
      .number = config->size << 1,
      .event = config->memory.event,
      .type = GPDMA_TYPE_P2M,
      .channel = config->memory.dma
  };

  interface->outer = init(GpDmaList, &dmaConfig);

  if (interface->outer != NULL)
  {
    dmaConfigure(interface->outer, &dmaSettings);
    dmaSetCallback(interface->outer, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static size_t setupPins(struct AdcDmaStream *interface, const PinNumber *pins)
{
  size_t index = 0;
  uint32_t enabled = 0;

  while (pins[index])
  {
    assert(index < ARRAY_SIZE(interface->pins));
    interface->pins[index] = adcConfigPin(&interface->base, pins[index]);

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    const unsigned int channel = interface->pins[index].channel;
    assert(!(enabled >> channel));

    enabled |= 1 << channel;
    ++index;
  }

  assert(enabled != 0);
  interface->base.control |= CR_SEL(enabled);
  interface->mask = enabled;

  return index;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaStreamConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL);
  assert(config->converter.event < ADC_EVENT_END);
  assert(config->converter.event != ADC_SOFTWARE);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  const struct AdcDmaStreamHandlerConfig streamConfig = {
      .parent = object,
      .size = config->size
  };

  struct AdcDmaStream * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->stream = init(AdcDmaStreamHandler, &streamConfig);
  if (interface->stream == NULL)
    return E_ERROR;

  /* Initialize input pins */
  interface->count = setupPins(interface, config->pins);

  if (!setupOuterChannel(interface, config))
  {
    deinit(interface->stream);
    return E_ERROR;
  }

  if (!setupInnerChannel(interface, config))
  {
    deinit(interface->outer);
    deinit(interface->stream);
    return E_ERROR;
  }
  resetInnerBuffers(interface);

  if (config->converter.event == ADC_BURST)
    interface->base.control |= CR_BURST;
  else
    interface->base.control |= CR_START(config->converter.event);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter, void *data)
{
  const struct AdcDmaStream * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      *(size_t *)data = pointerQueueCapacity(&interface->stream->requests)
          - pointerQueueSize(&interface->stream->requests);
      return E_OK;

    case IF_RX_PENDING:
      *(size_t *)data = pointerQueueSize(&interface->stream->requests);
      return E_OK;

    case IF_STATUS:
      return dmaStatus(interface->outer);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct AdcDmaStream * const interface = object;

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, NULL, object) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, NULL);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcHandlerInit(void *object, const void *configBase)
{
  const struct AdcDmaStreamHandlerConfig * const config = configBase;
  struct AdcDmaStreamHandler * const stream = object;

  if (pointerQueueInit(&stream->requests, config->size))
  {
    stream->parent = config->parent;
    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void adcHandlerClear(void *object)
{
  struct AdcDmaStreamHandler * const stream = object;

  while (!pointerQueueEmpty(&stream->requests))
  {
    struct StreamRequest * const request = pointerQueueFront(&stream->requests);
    pointerQueuePopFront(&stream->requests);

    request->callback(request->argument, request, STREAM_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcHandlerEnqueue(void *object,
    struct StreamRequest *request)
{
  struct AdcDmaStreamHandler * const stream = object;
  struct AdcDmaStream * const interface = stream->parent;

  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned with the sample size */
  assert(request->capacity / (interface->count * sizeof(uint16_t)) >= 2);
  assert(request->capacity % (interface->count * sizeof(uint16_t)) == 0);
  /* Output buffer should be aligned with the sample size */
  assert((uintptr_t)request->buffer % sizeof(uint16_t) == 0);

  /* Prepare linked list of DMA descriptors */
  const size_t samples = request->capacity / sizeof(uint16_t);
  const size_t parts[] = {samples >> 1, samples - (samples >> 1)};

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    LPC_ADC_Type * const reg = interface->base.reg;
    uint16_t * const dst = request->buffer;

    /* When a previous transfer is ongoing it will be continued */
    dmaAppend(interface->outer, dst, &interface->buffer, parts[0]);
    dmaAppend(interface->outer, dst + parts[0], &interface->buffer, parts[1]);

    if (dmaStatus(interface->outer) != E_BUSY)
    {
      if (dmaEnable(interface->outer) == E_OK)
      {
        /* Clear pending requests */
        for (size_t index = 0; index < interface->count; ++index)
          (void)reg->DR[interface->pins[index].channel];

        if (dmaEnable(interface->inner) == E_OK)
        {
          /* Enable DMA requests */
          reg->INTEN = interface->mask;
          /* Reconfigure peripheral and start the conversion */
          reg->CR = interface->base.control;
        }
        else
        {
          dmaDisable(interface->outer);
          res = E_INTERFACE;
        }
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
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDmaStream * const interface = object;

  deinit(interface->stream);
  deinit(interface->outer);
  deinit(interface->inner);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcHandlerDeinit(void *object)
{
  struct AdcDmaStreamHandler * const stream = object;
  pointerQueueDeinit(&stream->requests);
}
#endif
/*----------------------------------------------------------------------------*/
struct Stream *adcDmaStreamGetInput(struct AdcDmaStream *interface)
{
  return (struct Stream *)interface->stream;
}
