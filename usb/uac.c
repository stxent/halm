/*
 * uac.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/irq.h>
#include <halm/usb/uac.h>
#include <halm/usb/uac_base.h>
#include <halm/usb/uac_defs.h>
#include <halm/usb/usb_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb_trace.h>
#include <xcore/memory.h>
#include <limits.h>
#include <malloc.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT
#  define MEM_ALIGNMENT CONFIG_PLATFORM_USB_DEVICE_BUFFER_ALIGNMENT
#endif
/*----------------------------------------------------------------------------*/
struct Uac
{
  struct Interface base;

  /* Lower half of the driver */
  struct UacBase *driver;

  void (*callback)(void *);
  void *callbackArgument;

  /* Queue for OUT requests */
  PointerQueue rxRequestQueue;
  /* Pool for IN requests */
  PointerArray txRequestPool;
  /* Pool for feedback requests */
  PointerArray fbRequestPool;
  /* Pointer to the beginning of the request pool */
  void *requests;

  struct UsbEndpoint *fbDataEp;
  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;

  uint32_t *sampleRateArray;
  unsigned short sampleRateCount;
  unsigned short sampleRateIndex;

  size_t queuedRxBytes;
  size_t queuedTxBytes;

  /* Rate feedback multiplier in Q16.16 format */
  uint32_t feedback;
  /* Device suspended due to error or external request */
  bool suspended;

  struct
  {
    /* Sample Rate changed */
    bool rate;
    /* Start of Frame received */
    bool sof;
  } events;
};
/*----------------------------------------------------------------------------*/
static void audioDataReceived(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void audioDataSent(void *, struct UsbRequest *,
    enum UsbRequestStatus);
static void feedbackDataSent(void *, struct UsbRequest *,
    enum UsbRequestStatus);

static void *allocBufferMemory(size_t, size_t, size_t, size_t *);
static inline size_t getBufferSize(uint32_t);
static inline size_t getMaxBufferSize(uint32_t);
static uint32_t getMaxSampleRate(const struct Uac *);
static bool parseSampleRates(struct Uac *, const uint32_t *);
static bool resetEndpoints(struct Uac *);
static bool sendRateFeedback(struct Uac *);
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *, const void *);
static void interfaceDeinit(void *);
static void interfaceSetCallback(void *, void (*)(void *), void *);
static enum Result interfaceGetParam(void *, int, void *);
static enum Result interfaceSetParam(void *, int, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Uac = &(const struct InterfaceClass){
    .size = sizeof(struct Uac),
    .init = interfaceInit,
    .deinit = interfaceDeinit,

    .setCallback = interfaceSetCallback,
    .getParam = interfaceGetParam,
    .setParam = interfaceSetParam,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
static void audioDataReceived(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct Uac * const interface = argument;
  bool event = false;

  pointerQueuePushBack(&interface->rxRequestQueue, request);

  if (status == USB_REQUEST_COMPLETED)
  {
    interface->queuedRxBytes += request->length;
    event = true;
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    interface->suspended = true;
    usbTrace("uac: suspended in read callback");
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void audioDataSent(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct Uac * const interface = argument;
  bool error = false;

  if (status == USB_REQUEST_COMPLETED)
  {
    interface->queuedTxBytes -= request->length;
  }
  else if (status != USB_REQUEST_CANCELLED)
  {
    error = true;
  }

  pointerArrayPushBack(&interface->txRequestPool, request);

  if (error)
  {
    interface->suspended = true;
    usbTrace("uac: suspended in write callback");
  }
}
/*----------------------------------------------------------------------------*/
static void feedbackDataSent(void *argument, struct UsbRequest *request,
    enum UsbRequestStatus status)
{
  struct Uac * const interface = argument;

  pointerArrayPushBack(&interface->fbRequestPool, request);

  if (status != USB_REQUEST_COMPLETED && status != USB_REQUEST_CANCELLED)
  {
    interface->suspended = true;
    usbTrace("uac: suspended in feedback callback");
  }
}
/*----------------------------------------------------------------------------*/
static void *allocBufferMemory(size_t requestCount, size_t bufferCount,
    size_t bufferSize, size_t *padding)
{
  size_t requestMemorySize = requestCount * sizeof(struct UsbRequest);

#ifdef MEM_ALIGNMENT
  requestMemorySize += MEM_ALIGNMENT - 1;
  requestMemorySize -= requestMemorySize % MEM_ALIGNMENT;

  if (padding != NULL)
    *padding = requestMemorySize;

  return memalign(MEM_ALIGNMENT, requestMemorySize + bufferCount * bufferSize);
#else
  if (padding != NULL)
    *padding = requestMemorySize;

  return malloc(requestCount * sizeof(struct UsbRequest)
      + bufferCount * bufferSize);
#endif
}
/*----------------------------------------------------------------------------*/
static inline size_t getBufferSize(uint32_t rate)
{
  /*
   * Returns the size of the single buffer, 1 kHz isochronous packet rate,
   * stereo configuration with S16_LE samples.
   */
  return ((rate + 999) / 1000) * sizeof(int16_t) * 2;
}
/*----------------------------------------------------------------------------*/
static inline size_t getMaxBufferSize(uint32_t rate)
{
  size_t size = getBufferSize(rate);

#ifdef MEM_ALIGNMENT
  size += MEM_ALIGNMENT - 1;
  size -= size % MEM_ALIGNMENT;
#endif

  return size;
}
/*----------------------------------------------------------------------------*/
static uint32_t getMaxSampleRate(const struct Uac *interface)
{
  size_t maxSampleRate = 0;

  for (size_t index = 0; index < interface->sampleRateCount; ++index)
  {
    if (interface->sampleRateArray[index] > maxSampleRate)
      maxSampleRate = interface->sampleRateArray[index];
  }

  return maxSampleRate;
}
/*----------------------------------------------------------------------------*/
static bool parseSampleRates(struct Uac *interface, const uint32_t *rates)
{
  size_t sampleRateCount = 0;

  for (const uint32_t *entry = rates; *entry; ++entry)
    ++sampleRateCount;
  if (!sampleRateCount)
    return false;

  interface->sampleRateArray = malloc(sizeof(uint32_t) * sampleRateCount);
  if (interface->sampleRateArray == NULL)
    return false;
  assert(sampleRateCount <= USHRT_MAX);

  interface->sampleRateCount = (unsigned short)sampleRateCount;
  interface->sampleRateIndex = 0;

  for (size_t index = 0; index < sampleRateCount; ++index)
    interface->sampleRateArray[index] = rates[index];

  return true;
}
/*----------------------------------------------------------------------------*/
static bool resetEndpoints(struct Uac *interface)
{
  const size_t maxPacketSize = uacBaseGetPacketSize(interface->driver);
  bool completed = true;

  interface->suspended = true;

  /* Enable endpoints */
  if (interface->fbDataEp != NULL)
  {
    usbEpClear(interface->fbDataEp);
    usbEpEnable(interface->fbDataEp, ENDPOINT_TYPE_ISOCHRONOUS,
        UAC_FEEDBACK_EP_SIZE);
  }
  if (interface->txDataEp != NULL)
  {
    usbEpClear(interface->txDataEp);
    usbEpEnable(interface->txDataEp, ENDPOINT_TYPE_ISOCHRONOUS, maxPacketSize);
  }

  if (interface->rxDataEp != NULL)
  {
    usbEpClear(interface->rxDataEp);
    usbEpEnable(interface->rxDataEp, ENDPOINT_TYPE_ISOCHRONOUS, maxPacketSize);

    /* Fill OUT endpoint queue */
    while (!pointerQueueEmpty(&interface->rxRequestQueue))
    {
      struct UsbRequest * const request =
          pointerQueueFront(&interface->rxRequestQueue);
      pointerQueuePopFront(&interface->rxRequestQueue);

      if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
      {
        completed = false;
        pointerQueuePushBack(&interface->rxRequestQueue, request);
        break;
      }
    }
  }

  if (completed)
    interface->suspended = false;

  return completed;
}
/*----------------------------------------------------------------------------*/
static bool sendRateFeedback(struct Uac *interface)
{
  if (pointerArrayEmpty(&interface->fbRequestPool))
    return false;

  const bool hs = uacBaseGetUsbSpeed(interface->driver) == USB_HS;
  struct UsbRequest * const request =
      pointerArrayBack(&interface->fbRequestPool);

  const uint32_t currentSampleRate =
      interface->sampleRateArray[interface->sampleRateIndex];
  const uint64_t samplesPerSecondScaled =
      (uint64_t)currentSampleRate * interface->feedback;
  const uint32_t fractionOffset = 16 + RATE_FB_MUL
      - (hs ? RATE_FB_OFFSET_HS : RATE_FB_OFFSET);
  const uint32_t samplesPerFrameScaled =
      (uint32_t)((samplesPerSecondScaled * ((1 << RATE_FB_MUL) * 1000)
          + (1 << (RATE_FB_MUL - 1))) >> fractionOffset);
  IrqState state;

  /* Critical section */
  state = irqSave();
  pointerArrayPopBack(&interface->fbRequestPool);
  irqRestore(state);

  const uint32_t buffer = toLittleEndian32(samplesPerFrameScaled);

  if (hs)
  {
    /* Q16.16 format, 4 bytes */
    memcpy(request->buffer, &buffer, 4);
  }
  else
  {
    /* Q10.14 format, 3 bytes */
    memcpy(request->buffer, &buffer, 3);
  }

  if (usbEpEnqueue(interface->fbDataEp, request) != E_OK)
  {
    /* Hardware error occurred, suspend the driver and wait for reset */
    interface->suspended = true;

    state = irqSave();
    pointerArrayPushBack(&interface->fbRequestPool, request);
    irqRestore(state);

    usbTrace("uac: suspended in feedback function");
    return false;
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *object, const void *configBase)
{
  const struct UacConfig * const config = configBase;
  assert(config != NULL);
  assert(config->device != NULL);
  assert(config->rates != NULL);

  struct Uac * const interface = object;

  if (!parseSampleRates(interface, config->rates))
    return E_VALUE;

  if (config->endpoints.fb)
  {
    interface->fbDataEp = usbDevCreateEndpoint(config->device,
        config->endpoints.fb);
    if (!interface->fbDataEp)
      return E_ERROR;

    if (!pointerArrayInit(&interface->fbRequestPool, 1))
      return E_MEMORY;
  }
  else
    interface->fbDataEp = NULL;

  if (config->endpoints.rx)
  {
    interface->rxDataEp = usbDevCreateEndpoint(config->device,
        config->endpoints.rx);
    if (!interface->rxDataEp)
      return E_ERROR;

    if (!pointerQueueInit(&interface->rxRequestQueue, config->rxBuffers))
      return E_MEMORY;
  }
  else
    interface->rxDataEp = NULL;

  if (config->endpoints.tx)
  {
    interface->txDataEp = usbDevCreateEndpoint(config->device,
        config->endpoints.tx);
    if (!interface->txDataEp)
      return E_ERROR;

    if (!pointerArrayInit(&interface->txRequestPool, config->txBuffers))
      return E_MEMORY;
  }
  else
    interface->txDataEp = NULL;

  interface->callback = NULL;
  interface->callbackArgument = NULL;
  interface->queuedRxBytes = 0;
  interface->queuedTxBytes = 0;
  interface->feedback = 1 << 16;
  interface->suspended = true;
  interface->events.rate = false;
  interface->events.sof = false;

  const size_t size = getMaxBufferSize(getMaxSampleRate(interface));
  const size_t fbBuffers = interface->fbDataEp != NULL ? 1 : 0;
  const size_t rxBuffers = interface->rxDataEp != NULL ? config->rxBuffers : 0;
  const size_t txBuffers = interface->txDataEp != NULL ? config->txBuffers : 0;
  const size_t count = fbBuffers + rxBuffers + txBuffers;
  uint8_t *arena;

  /* Allocate requests */
  if (config->arena)
  {
    interface->requests = allocBufferMemory(count, 0, 0, NULL);
    if (!interface->requests)
      return E_MEMORY;

    arena = config->arena;
  }
  else
  {
    size_t padding;

    interface->requests = allocBufferMemory(count, count, size, &padding);
    if (!interface->requests)
      return E_MEMORY;

    arena = (uint8_t *)interface->requests + padding;
  }

  /* Add requests to queues */
  struct UsbRequest *request = interface->requests;
  uint8_t *payload = arena;

  for (size_t index = 0; index < fbBuffers; ++index)
  {
    usbRequestInit(request, payload, size, feedbackDataSent, interface);
    pointerArrayPushBack(&interface->fbRequestPool, request);

    ++request;
    payload += size; // TODO Reduce FB request size
  }

  for (size_t index = 0; index < rxBuffers; ++index)
  {
    usbRequestInit(request, payload, size, audioDataReceived, interface);
    pointerQueuePushBack(&interface->rxRequestQueue, request);

    ++request;
    payload += size;
  }

  for (size_t index = 0; index < txBuffers; ++index)
  {
    usbRequestInit(request, payload, size, audioDataSent, interface);
    pointerArrayPushBack(&interface->txRequestPool, request);

    ++request;
    payload += size;
  }

  /* Lower half of the driver should be initialized after all other parts */
  const struct UacBaseConfig driverConfig = {
      .owner = interface,
      .device = config->device,
      .endpoints = {
          .fb = config->endpoints.fb,
          .rx = config->endpoints.rx,
          .tx = config->endpoints.tx
      },
      .packet = size
  };

  interface->driver = init(UacBase, &driverConfig);
  return interface->driver != NULL ? E_OK : E_ERROR;
}
/*----------------------------------------------------------------------------*/
static void interfaceDeinit(void *object)
{
  struct Uac * const interface = object;

  /* Call destructor for USB driver part */
  deinit(interface->driver);

  /* Return requests from endpoint queues to local pools */
  if (interface->txDataEp != NULL)
    usbEpClear(interface->txDataEp);
  if (interface->rxDataEp != NULL)
    usbEpClear(interface->rxDataEp);
  if (interface->fbDataEp != NULL)
    usbEpClear(interface->fbDataEp);

  assert(pointerArrayFull(&interface->fbRequestPool));
  assert(pointerQueueFull(&interface->rxRequestQueue));
  assert(pointerArrayFull(&interface->txRequestPool));

  /* Free memory allocated for request buffers */
  free(interface->requests);

  /* Delete endpoints and request pools */
  if (interface->txDataEp != NULL)
  {
    deinit(interface->txDataEp);
    pointerArrayDeinit(&interface->txRequestPool);
  }
  if (interface->rxDataEp != NULL)
  {
    deinit(interface->rxDataEp);
    pointerQueueDeinit(&interface->rxRequestQueue);
  }
  if (interface->fbDataEp != NULL)
  {
    deinit(interface->fbDataEp);
    pointerArrayDeinit(&interface->fbRequestPool);
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Uac * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceGetParam(void *object, int parameter, void *data)
{
  struct Uac * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      *(size_t *)data = interface->suspended ? 0 : interface->queuedRxBytes;
      return E_OK;

    case IF_RX_PENDING:
      if (!interface->suspended)
      {
        const size_t buffers = pointerQueueCapacity(&interface->rxRequestQueue)
            - pointerQueueSize(&interface->rxRequestQueue);
        *(size_t *)data = buffers * uacBaseGetPacketSize(interface->driver);
      }
      else
        *(size_t *)data = 0;
      return E_OK;

    case IF_TX_AVAILABLE:
      if (!interface->suspended)
      {
        const size_t buffers = pointerArraySize(&interface->txRequestPool);
        *(size_t *)data = buffers * uacBaseGetPacketSize(interface->driver);
      }
      else
        *(size_t *)data = 0;
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = interface->suspended ? 0 : interface->queuedTxBytes;
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data =
          interface->sampleRateArray[interface->sampleRateIndex];
      return E_OK;

    default:
      break;
  }

  switch ((enum UacParameter)parameter)
  {
    case IF_UAC_STATUS:
    {
      uint8_t status = 0;

      if (interface->events.sof)
      {
        interface->events.sof = false;
        status |= UAC_SOF;
      }
      if (interface->events.rate)
      {
        interface->events.rate = false;
        status |= UAC_RATE;
      }

      if (interface->suspended)
      {
        status |= UAC_SUSPENDED;
      }
      else
      {
        if (uacBaseIsRxActive(interface->driver))
          status |= UAC_RX_ACTIVE;
        if (uacBaseIsTxActive(interface->driver))
          status |= UAC_TX_ACTIVE;

        if (interface->queuedRxBytes)
          status |= UAC_RX_READY;
        if (!interface->queuedTxBytes)
          status |= UAC_TX_EMPTY;
      }

      *(uint8_t *)data = status;
      return E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceSetParam(void *object, int parameter,
    const void *data)
{
  struct Uac * const interface = object;

  switch ((enum UacParameter)parameter)
  {
    case IF_UAC_FEEDBACK:
      interface->feedback = *(const uint32_t *)data;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct Uac * const interface = object;
  uint8_t *bufferPosition = buffer;

  assert(length >= getBufferSize(
      interface->sampleRateArray[interface->sampleRateIndex]));

  if (interface->suspended || interface->rxDataEp == NULL)
    return 0;
  if (!uacBaseIsRxActive(interface->driver))
    return 0;

  while (!pointerQueueEmpty(&interface->rxRequestQueue))
  {
    struct UsbRequest * const request =
        pointerQueueFront(&interface->rxRequestQueue);

    if (length < request->length)
      break;

    const size_t bytesToRead = request->length;
    IrqState state;

    state = irqSave();
    pointerQueuePopFront(&interface->rxRequestQueue);
    interface->queuedRxBytes -= bytesToRead;
    irqRestore(state);

    memcpy(bufferPosition, request->buffer, bytesToRead);
    bufferPosition += bytesToRead;
    length -= bytesToRead;

    if (usbEpEnqueue(interface->rxDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the driver and wait for reset */
      interface->suspended = true;

      state = irqSave();
      pointerQueuePushBack(&interface->rxRequestQueue, request);
      irqRestore(state);

      usbTrace("uac: suspended in read function");
      break;
    }
  }

  /* Send rate feedback */
  if (interface->fbDataEp != NULL)
    sendRateFeedback(interface);

  return bufferPosition - (uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct Uac * const interface = object;
  const uint8_t *bufferPosition = buffer;
  const size_t maxPacketSize = uacBaseGetPacketSize(interface->driver);

  if (interface->suspended || interface->txDataEp == NULL)
    return 0;
  if (!uacBaseIsTxActive(interface->driver))
    return 0;

  while (length && !pointerArrayEmpty(&interface->txRequestPool))
  {
    const size_t bytesToWrite = MIN(length, maxPacketSize);
    struct UsbRequest *request;
    IrqState state;

    /* Critical section */
    state = irqSave();
    request = pointerArrayBack(&interface->txRequestPool);
    pointerArrayPopBack(&interface->txRequestPool);
    interface->queuedTxBytes += bytesToWrite;
    irqRestore(state);

    request->length = bytesToWrite;
    memcpy(request->buffer, bufferPosition, bytesToWrite);

    if (usbEpEnqueue(interface->txDataEp, request) != E_OK)
    {
      /* Hardware error occurred, suspend the driver and wait for reset */
      interface->suspended = true;

      state = irqSave();
      pointerArrayPushBack(&interface->txRequestPool, request);
      irqRestore(state);

      usbTrace("uac: suspended in write function");
      break;
    }

    bufferPosition += bytesToWrite;
    length -= bytesToWrite;
  }

  return bufferPosition - (const uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
void uacOnEvent(struct Uac *interface, unsigned int event)
{
  switch ((enum UsbDeviceEvent)event)
  {
    case USB_DEVICE_EVENT_FRAME:
      interface->events.sof = true;
      break;

    case USB_DEVICE_EVENT_RESET:
      if (resetEndpoints(interface))
      {
        interface->sampleRateIndex = 0;
        interface->suspended = false;
        interface->events.rate = true;
        interface->events.sof = false;
        usbTrace("uac: reset completed");
      }
      else
        usbTrace("uac: reset failed");
      break;

    case USB_DEVICE_EVENT_SUSPEND:
      interface->suspended = true;
      usbTrace("uac: suspended");
      break;

    case USB_DEVICE_EVENT_RESUME:
      interface->suspended = false;
      usbTrace("uac: resumed");
      break;

    default:
      break;
  }

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
uint32_t uacOnSampleRateGet(const struct Uac *interface, size_t index)
{
  return (index < interface->sampleRateCount) ?
      interface->sampleRateArray[index] : 0;
}
/*----------------------------------------------------------------------------*/
uint32_t uacOnSampleRateGetCurrent(const struct Uac *interface)
{
  return interface->sampleRateArray[interface->sampleRateIndex];
}
/*----------------------------------------------------------------------------*/
bool uacOnSampleRateSet(struct Uac *interface, uint32_t rate)
{
  for (size_t index = 0; index < interface->sampleRateCount; ++index)
  {
    if (interface->sampleRateArray[index] == rate)
    {
      interface->events.rate = true;
      interface->sampleRateIndex = index;

      if (interface->callback != NULL)
        interface->callback(interface->callbackArgument);
      return true;
    }
  }

  return false;
}
