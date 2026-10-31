/*
 * dcmi.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/stm32/dcmi.h>
#include <halm/platform/stm32/dcmi_defs.h>
#include <halm/platform/stm32/dma_list.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct DcmiStreamConfig
{
  /** Mandatory: pointer to a parent object. */
  struct Dcmi *parent;
  /** Mandatory: queue size. */
  size_t size;
};

struct DcmiStream
{
  struct Stream base;

  /* Parent interface */
  struct Dcmi *parent;
  /* Request queue */
  PointerQueue requests;
};
/*----------------------------------------------------------------------------*/
static void cleanupInterface(struct Dcmi *);
static bool dcmiStart(struct Dcmi *);
static void dcmiStop(struct Dcmi *);
static bool dmaSetup(struct Dcmi *, const struct DcmiConfig *);
/*----------------------------------------------------------------------------*/
static enum Result dcmiInit(void *, const void *);
static void dcmiSetCallback(void *, void (*)(void *), void *);
static enum Result dcmiGetParam(void *, int, void *);
static enum Result dcmiSetParam(void *, int, const void *);

static enum Result dcmiStreamInit(void *, const void *);
static void dcmiStreamClear(void *);
static enum Result dcmiStreamEnqueue(void *, struct StreamRequest *);

#ifndef CONFIG_PLATFORM_STM32_DCMI_NO_DEINIT
static void dcmiDeinit(void *);
static void dcmiStreamDeinit(void *);
#else
#  define dcmiDeinit deletedDestructorTrap
#  define dcmiStreamDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Dcmi = &(const struct InterfaceClass){
    .size = sizeof(struct Dcmi),
    .init = dcmiInit,
    .deinit = dcmiDeinit,

    .setCallback = dcmiSetCallback,
    .getParam = dcmiGetParam,
    .setParam = dcmiSetParam,
    .read = nullptr,
    .write = nullptr
};

const struct StreamClass * const DcmiStream = &(const struct StreamClass){
    .size = sizeof(struct DcmiStream),
    .init = dcmiStreamInit,
    .deinit = dcmiStreamDeinit,

    .clear = dcmiStreamClear,
    .enqueue = dcmiStreamEnqueue
};
/*----------------------------------------------------------------------------*/
static void dcmiHandler(void *object)
{
  struct Dcmi * const interface = object;
  struct DcmiStream * const stream = interface->stream;
  STM_DCMI_Type * const reg = interface->base.reg;
  const uint32_t status = reg->MISR;

  if (status & MISR_OVR_MIS)
  {
    reg->ICR = ICR_OVR_ISC;
    interface->error = true;
  }

  if (status & MISR_FRAME_MIS)
  {
    reg->ICR = ICR_FRAME_ISC;
    interface->error = false;

    if (!pointerQueueEmpty(&stream->requests))
    {
      struct StreamRequest *request = pointerQueueFront(&stream->requests);
      size_t residue;

      if (dmaResidue(interface->dma, &residue)
          && residue != request->capacity)
      {
        /* Retire partially completed request */
        dmaDisable(interface->dma);

        pointerQueuePopFront(&stream->requests);
        request->length = request->capacity - residue;
        request->callback(request->argument, request, STREAM_REQUEST_COMPLETED);
      }
    }
  }

  if (interface->callback != nullptr)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct Dcmi * const interface = object;
  struct DcmiStream * const stream = interface->stream;
  const enum Result transferStatus = dmaStatus(interface->dma);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;

  if (transferStatus == E_ERROR)
    requestStatus = STREAM_REQUEST_FAILED;

  struct StreamRequest * const request =
      pointerQueueFront(&stream->requests);

  pointerQueuePopFront(&stream->requests);
  request->length = request->capacity;
  request->callback(request->argument, request, requestStatus);

  if (requestStatus == STREAM_REQUEST_FAILED || transferStatus != E_BUSY)
  {
    /* DMA stream is aborted, all remaining requests are cancelled */
    while (!pointerQueueEmpty(&stream->requests))
    {
      struct StreamRequest * const pending =
          pointerQueueFront(&stream->requests);

      pointerQueuePopFront(&stream->requests);
      pending->callback(pending->argument, pending, STREAM_REQUEST_CANCELLED);
    }
  }
}
/*----------------------------------------------------------------------------*/
static void cleanupInterface(struct Dcmi *interface)
{
  if (interface->stream != nullptr)
    deinit(interface->stream);
  if (interface->dma != nullptr)
    deinit(interface->dma);
  DcmiBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static bool dcmiStart(struct Dcmi *interface)
{
  STM_DCMI_Type * const reg = interface->base.reg;
  uint32_t cr = reg->CR & ~(CR_CM | CR_JPEG);

  if (dmaEnable(interface->dma) != E_OK)
  {
    interface->error = true;
    return false;
  }

  if (!interface->continuous)
    cr |= CR_CM;
  if (!interface->jpeg)
    cr |= CR_JPEG;
  cr |= CR_CAPTURE | CR_ENABLE;

  interface->error = false;
  reg->CR = cr;

  return true;
}
/*----------------------------------------------------------------------------*/
static void dcmiStop(struct Dcmi *interface)
{
  STM_DCMI_Type * const reg = interface->base.reg;

  reg->CR &= ~(CR_ENABLE | CR_CAPTURE);
  dmaDisable(interface->dma);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Dcmi *interface,
    const struct DcmiConfig *config)
{
  interface->dma = dcmiMakeListDma(config->size, config->dma,
      config->dmaPriority);

  if (interface->dma != nullptr)
  {
    const struct DmaSettings dmaSettings = {
        .source = {
            .burst = DMA_BURST_1,
            .width = DMA_WIDTH_WORD,
            .increment = false
        },
        .destination = {
            .burst = DMA_BURST_4,
            .width = DMA_WIDTH_WORD,
            .increment = true
        }
    };

    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiInit(void *object, const void *configBase)
{
  const struct DcmiConfig * const config = configBase;
  assert(config != nullptr);
  assert(config->rate <= DCMI_ALT_4_FRAME);
  assert(config->size != 0);
  assert(config->width == 8 || config->width == 10
      || config->width == 12 || config->width == 14);

  const struct DcmiBaseConfig baseConfig = {
      .pins = {
          .data = config->pins.data,
          .hsync = config->pins.hsync,
          .vsync = config->pins.vsync,
          .pixclk = config->pins.pixclk
      }
  };
  const struct DcmiStreamConfig streamConfig = {
      .parent = object,
      .size = config->size
  };
  struct Dcmi * const interface = object;

  const enum Result res = DcmiBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->base.handler = dcmiHandler;

  interface->callback = nullptr;
  interface->callbackArgument = nullptr;
  interface->dma = nullptr;
  interface->stream = nullptr;
  interface->continuous = config->continuous;
  interface->error = false;
  interface->jpeg = config->jpeg;

  STM_DCMI_Type * const reg = interface->base.reg;

  const bool hasHardwareSync = config->pins.hsync && config->pins.vsync;
  const uint8_t edm = (config->width >> 1) & 0x3;

  reg->CR = (hasHardwareSync ? 0 : CR_ESS)
      | (config->polarity.pixclk ? CR_PCKPOL : 0)
      | (config->polarity.vsync ? CR_VSPOL : 0)
      | (config->polarity.hsync ? CR_HSPOL : 0)
      | CR_FCRC(config->rate)
      | CR_EDM(edm);

  reg->ICR = ICR_MASK;
  reg->IER = IER_FRAME_IE | IER_OVR_IE;

  if (!dmaSetup(interface, config))
  {
    cleanupInterface(interface);
    return E_ERROR;
  }

  interface->stream = init(DcmiStream, &streamConfig);
  if (interface->stream == nullptr)
  {
    cleanupInterface(interface);
    return E_ERROR;
  }

  /* Enable interrupts in the NVIC */
  irqSetPriority(interface->base.irq, config->irqPriority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_DCMI_NO_DEINIT
static void dcmiDeinit(void *object)
{
  struct Dcmi * const interface = object;

  dcmiStop(interface);
  irqDisable(interface->base.irq);

  cleanupInterface(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void dcmiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Dcmi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiGetParam(void *object, int parameter, void *value)
{
  const struct Dcmi * const interface = object;

  switch ((enum DcmiParameter)parameter)
  {
    case IF_DCMI_SNAPSHOT:
      *(uint8_t *)value = interface->continuous ? 1 : 0;
      return E_OK;

    case IF_DCMI_JPEG:
      *(uint8_t *)value = interface->jpeg ? 1 : 0;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return interface->error ? E_INTERFACE : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiSetParam(void *object, int parameter, const void *value)
{
  struct Dcmi * const interface = object;

  switch ((enum DcmiParameter)parameter)
  {
    case IF_DCMI_SNAPSHOT:
      interface->continuous = *(const uint8_t *)value != 0;
      return E_OK;

    case IF_DCMI_JPEG:
      interface->jpeg = *(const uint8_t *)value != 0;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      dcmiStop(interface);
      return E_OK;

    case IF_ENABLE:
      return dcmiStart(interface) ? E_OK : E_INTERFACE;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiStreamInit(void *object, const void *configBase)
{
  const struct DcmiStreamConfig * const config = configBase;
  struct DcmiStream * const stream = object;

  if (pointerQueueInit(&stream->requests, config->size))
  {
    stream->parent = config->parent;
    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void dcmiStreamDeinit(void *object)
{
  struct DcmiStream * const stream = object;
  pointerQueueDeinit(&stream->requests);
}
/*----------------------------------------------------------------------------*/
static void dcmiStreamClear(void *object)
{
  struct DcmiStream * const stream = object;
  struct Dcmi * const parent = stream->parent;
  const IrqState state = irqSave();

  if (dmaStatus(parent->dma) == E_BUSY)
    dcmiStop(parent);
  dmaClear(parent->dma);

  irqRestore(state);

  while (!pointerQueueEmpty(&stream->requests))
  {
    struct StreamRequest * const request = pointerQueueFront(&stream->requests);
    pointerQueuePopFront(&stream->requests);

    request->callback(request->argument, request, STREAM_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result dcmiStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  assert(request != nullptr && request->callback != nullptr);
  /* The buffer is aligned on the word size */
  assert(request->capacity % sizeof(uint32_t) == 0);
  assert(!(((uintptr_t)request->buffer) % sizeof(uint32_t)));

  struct DcmiStream * const stream = object;
  struct Dcmi * const parent = stream->parent;

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    const STM_DCMI_Type * const reg = parent->base.reg;

    dmaAppend(parent->dma, request->buffer, (const void *)&reg->DR,
        request->capacity);
  }
  else
    res = E_FULL;

  if (res == E_OK)
    pointerQueuePushBack(&stream->requests, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
struct Stream *dcmiGetInput(struct Dcmi *interface)
{
  return &interface->stream->base;
}
