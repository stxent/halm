/*
 * buffering_proxy.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/buffering_proxy.h>
#include <halm/irq.h>
#include <assert.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
static bool enqueueRxRequests(struct BufferingProxy *);
static void onRxStreamEvent(void *, struct StreamRequest *,
    enum StreamRequestStatus);
static void onTxStreamEvent(void *, struct StreamRequest *,
    enum StreamRequestStatus);
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *, const void *);
static void interfaceDeinit(void *);
static void interfaceSetCallback(void *, void (*)(void *), void *);
static enum Result interfaceGetParam(void *, int, void *);
static enum Result interfaceSetParam(void *, int, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const BufferingProxy =
    &(const struct InterfaceClass){
    .size = sizeof(struct BufferingProxy),
    .init = interfaceInit,
    .deinit = interfaceDeinit,

    .setCallback = interfaceSetCallback,
    .getParam = interfaceGetParam,
    .setParam = interfaceSetParam,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
static bool enqueueRxRequests(struct BufferingProxy *interface)
{
  bool completed = true;

  while (!pointerArrayEmpty(&interface->rxPool))
  {
    struct StreamRequest * const request = pointerArrayBack(&interface->rxPool);
    pointerArrayPopBack(&interface->rxPool);

    if (streamEnqueue(interface->rx, request) != E_OK)
    {
      completed = false;
      pointerArrayPushBack(&interface->rxPool, request);
      break;
    }
  }

  return completed;
}
/*----------------------------------------------------------------------------*/
static void onRxStreamEvent(void *argument, struct StreamRequest *request,
    enum StreamRequestStatus status)
{
  struct BufferingProxy * const interface = argument;

  if (status == STREAM_REQUEST_COMPLETED)
  {
    pointerQueuePushBack(&interface->rxQueue, request);

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
  else
    streamEnqueue(interface->rx, request);
}
/*----------------------------------------------------------------------------*/
static void onTxStreamEvent(void *argument, struct StreamRequest *request,
    enum StreamRequestStatus status)
{
  struct BufferingProxy * const interface = argument;

  pointerArrayPushBack(&interface->txPool, request);

  if (status == STREAM_REQUEST_COMPLETED && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *object, const void *configBase)
{
  const struct BufferingProxyConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pipe != NULL);

  struct BufferingProxy * const interface = object;

  interface->callback = NULL;
  interface->pipe = config->pipe;
  interface->rx = config->rx.stream;
  interface->tx = config->tx.stream;

  if (interface->rx != NULL)
  {
    interface->rxBufferCount = config->rx.count;
    interface->rxBufferSize = config->rx.size;
  }
  else
  {
    interface->rxBufferCount = 0;
    interface->rxBufferSize = 0;
  }

  if (!pointerQueueInit(&interface->rxQueue, interface->rxBufferCount))
    return E_MEMORY;
  if (!pointerArrayInit(&interface->rxPool, interface->rxBufferCount))
    return E_MEMORY;

  if (interface->tx != NULL)
  {
    interface->txBufferCount = config->tx.count;
    interface->txBufferSize = config->tx.size;
  }
  else
  {
    interface->txBufferCount = 0;
    interface->txBufferSize = 0;
  }

  if (!pointerArrayInit(&interface->txPool, interface->txBufferCount))
    return E_MEMORY;

  const size_t rxSize = sizeof(struct StreamRequest) + interface->rxBufferSize;
  const size_t txSize = sizeof(struct StreamRequest) + interface->txBufferSize;
  const size_t poolSize =
      rxSize * interface->rxBufferCount + txSize * interface->txBufferCount;
  uint8_t *arena = malloc(poolSize);

  if (arena == NULL)
    return E_MEMORY;
  interface->arena = arena;

  for (size_t index = 0; index < interface->rxBufferCount; ++index)
  {
    struct StreamRequest * const request = (struct StreamRequest *)arena;

    request->capacity = interface->rxBufferSize;
    request->length = 0;
    request->callback = onRxStreamEvent;
    request->argument = interface;
    request->buffer = arena + sizeof(struct StreamRequest);

    pointerArrayPushBack(&interface->rxPool, (void *)request);
    arena += rxSize;
  }
  for (size_t index = 0; index < interface->txBufferCount; ++index)
  {
    struct StreamRequest * const request = (struct StreamRequest *)arena;

    request->capacity = interface->txBufferSize;
    request->length = 0;
    request->callback = onTxStreamEvent;
    request->argument = interface;
    request->buffer = arena + sizeof(struct StreamRequest);

    pointerArrayPushBack(&interface->txPool, (void *)request);
    arena += txSize;
  }

  enqueueRxRequests(interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interfaceDeinit(void *object)
{
  struct BufferingProxy * const interface = object;

  free(interface->arena);
  pointerArrayDeinit(&interface->txPool);
  pointerArrayDeinit(&interface->rxPool);
  pointerQueueDeinit(&interface->rxQueue);
}
/*----------------------------------------------------------------------------*/
static void interfaceSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct BufferingProxy * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceGetParam(void *object, int parameter, void *data)
{
  struct BufferingProxy * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      if (interface->rx == NULL)
        return E_INVALID;

      *(size_t *)data = pointerQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_RX_PENDING:
      if (interface->rx == NULL)
        return E_INVALID;

      *(size_t *)data = interface->rxBufferCount
          - pointerQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_TX_AVAILABLE:
      if (interface->tx == NULL)
        return E_INVALID;

      *(size_t *)data = pointerArraySize(&interface->txPool);
      return E_OK;

    case IF_TX_PENDING:
      if (interface->tx == NULL)
        return E_INVALID;

      *(size_t *)data = interface->txBufferCount
          - pointerArraySize(&interface->txPool);
      return E_OK;

    default:
      break;
  }

  return ifGetParam(interface->pipe, parameter, data);
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceSetParam(void *object, int parameter,
    const void *data)
{
  struct BufferingProxy * const interface = object;
  return ifSetParam(interface->pipe, parameter, data);
}
/*----------------------------------------------------------------------------*/
static size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct BufferingProxy * const interface = object;
  uint8_t *bufferPosition = buffer;

  assert(length >= interface->rxBufferSize);

  if (!pointerQueueEmpty(&interface->rxQueue))
  {
    struct StreamRequest * const request =
        pointerQueueFront(&interface->rxQueue);

    const size_t bytesToRead = request->length;
    IrqState state;

    state = irqSave();
    pointerQueuePopFront(&interface->rxQueue);
    irqRestore(state);

    memcpy(bufferPosition, request->buffer, bytesToRead);
    bufferPosition += bytesToRead;
    length -= bytesToRead;

    if (streamEnqueue(interface->rx, request) != E_OK)
    {
      state = irqSave();
      pointerArrayPushBack(&interface->rxPool, request);
      irqRestore(state);
    }
  }

  return bufferPosition - (uint8_t *)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct BufferingProxy * const interface = object;
  const uint8_t *bufferPosition = buffer;

  assert(length <= interface->txBufferSize);

  if (length && !pointerArrayEmpty(&interface->txPool))
  {
    const size_t bytesToWrite = MIN(length, interface->txBufferSize);
    struct StreamRequest *request;
    IrqState state;

    state = irqSave();
    request = pointerArrayBack(&interface->txPool);
    pointerArrayPopBack(&interface->txPool);
    irqRestore(state);

    request->length = bytesToWrite;
    memcpy(request->buffer, bufferPosition, bytesToWrite);

    if (streamEnqueue(interface->tx, request) == E_OK)
    {
      bufferPosition += bytesToWrite;
      length -= bytesToWrite;
    }
    else
    {
      state = irqSave();
      pointerArrayPushBack(&interface->txPool, request);
      irqRestore(state);
    }
  }

  return bufferPosition - (const uint8_t *)buffer;
}
