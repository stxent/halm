/*
 * adc_dma_stream.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/adc_dma_stream.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <halm/platform/lpc/sdma_list.h>
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
static bool dmaSetup(struct AdcDmaStream *, uint8_t, uint8_t, size_t);
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
#  define adcDeinit deletedDestructorTrap
#  define adcHandlerDeinit deletedDestructorTrap
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
  const enum Result transferStatus = dmaStatus(interface->dma);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;
  bool event = false;

  /* Scatter-gather transfer finished */
  if (transferStatus != E_BUSY)
  {
    const unsigned int number = interface->base.sequence & 1;
    LPC_ADC_Type * const reg = interface->base.reg;

    /* Stop further conversions */
    reg->SEQ_CTRL[number] &= ~SEQ_CTRL_SEQ_ENA;

    /* Disable DMA requests */
    const IrqState state = irqSave();
    reg->INTEN &= ~INTEN_SEQ_INTEN(number);
    irqRestore(state);

    if (transferStatus == E_ERROR)
      requestStatus = STREAM_REQUEST_FAILED;

    event = true;
  }
  else if ((dmaQueued(interface->dma) & 1) == 0)
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
static bool dmaSetup(struct AdcDmaStream *interface, uint8_t channel,
    uint8_t priority, size_t size)
{
  static const struct SdmaSettings dmaSettings = {
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .source = {
          .stride = SDMA_STRIDE_NONE,
          .wrap = true
      },
      .destination = {
          .stride = SDMA_STRIDE_1,
          .wrap = false
      }
  };
  const struct SdmaListConfig dmaConfig = {
      .number = size << 1,
      .request = SDMA_REQUEST_NONE,
      .trigger = sdmaGetTriggerAdc(interface->base.sequence),
      .channel = channel,
      .priority = priority,
      .polarity = true
  };

  interface->dma = init(SdmaList, &dmaConfig);

  if (interface->dma != NULL)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaStreamConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);
  assert(config->event < ADC_EVENT_END);
  assert(!config->preemption || (config->sequence & 1) == 0);
  assert(config->sensitivity <= INPUT_FALLING);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .sequence = config->sequence,
      .accuracy = config->accuracy,
      .shared = config->shared
  };
  const struct AdcDmaStreamHandlerConfig streamConfig = {
      .parent = object,
      .size = config->size
  };

  struct AdcDmaStream * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  if (!dmaSetup(interface, config->dma, config->priority, config->size))
    return E_ERROR;

  interface->stream = init(AdcDmaStreamHandler, &streamConfig);
  if (interface->stream == NULL)
    return E_ERROR;

  interface->pins = malloc(sizeof(struct AdcPin) * count);
  if (interface->pins == NULL)
    return E_MEMORY;

  interface->count = (uint8_t)count;

  interface->control = 0;
  if (config->event == ADC_BURST)
    interface->control |= SEQ_CTRL_BURST;
  else
    interface->control |= SEQ_CTRL_TRIGGER(config->event);
  if (config->preemption)
    interface->control |= SEQ_CTRL_LOWPRIO;
  if (config->sensitivity == INPUT_RISING)
    interface->control |= SEQ_CTRL_TRIGPOL;
  if (config->singlestep)
    interface->control |= SEQ_CTRL_SINGLESTEP;

  /* Initialize input pins */
  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->control |= SEQ_CTRL_CHANNELS(mask);
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
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct AdcDmaStream * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      adcStartCalibration(&interface->base);
      return E_OK;

    default:
      break;
  }

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  switch ((enum IfParameter)parameter)
  {
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.sequence, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.sequence, &interface->base, NULL);
      return E_OK;

    default:
      break;
  }
#endif

  return E_INVALID;
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

  assert(adcGetInstance(interface->base.sequence) == &interface->base);
  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned on the sample size */
  assert(request->capacity / (interface->count * sizeof(uint16_t)) >= 2);
  assert(request->capacity % (interface->count * sizeof(uint16_t)) == 0);

  /* Prepare linked list of DMA descriptors */
  const size_t parts[] = {
      request->capacity >> 1,
      request->capacity - (request->capacity >> 1)
  };

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    const unsigned int number = interface->base.sequence & 1;
    LPC_ADC_Type * const reg = interface->base.reg;
    uint8_t * const destination = request->buffer;

    /* When a previous transfer is active it will be continued */
    dmaAppend(interface->dma, destination,
        (const void *)&reg->SEQ_GDAT[number], parts[0]);
    dmaAppend(interface->dma, destination + parts[0],
        (const void *)&reg->SEQ_GDAT[number], parts[1]);

    if (dmaStatus(interface->dma) != E_BUSY)
    {
      /* Configure the sequence, but don't enable it */
      reg->SEQ_CTRL[number] = interface->control;

      /* Clear pending request */
      reg->FLAGS = FLAGS_SEQ_INT(number);

      if (dmaEnable(interface->dma) == E_OK)
      {
        /* Enable DMA request after each conversion */
        reg->INTEN |= INTEN_SEQ_INTEN(number);

        /* Reconfigure peripheral and start the conversion */
        reg->SEQ_CTRL[number] |= SEQ_CTRL_SEQ_ENA;
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

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->pins);

  deinit(interface->stream);
  deinit(interface->dma);
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
