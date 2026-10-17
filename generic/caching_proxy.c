/*
 * caching_proxy.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/caching_proxy.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
enum
{
  FLAG_DIRTY = 0x01
};

struct CachedBlock
{
  uint32_t address;
  uint32_t flags;
  uint32_t timestamp;
  void *data;
};

#define BLOCK_RESERVED UINT32_MAX
/*----------------------------------------------------------------------------*/
static struct CachedBlock *allocateBlock(struct CachingProxy *);
static struct CachedBlock *findBlock(struct CachingProxy *, uint32_t);
static struct CachedBlock *findEmptyBlock(struct CachingProxy *);
static struct CachedBlock *findOldestBlock(struct CachingProxy *);
static enum Result flushAll(struct CachingProxy *);
static enum Result flushBlock(struct CachingProxy *, struct CachedBlock *);
static void invalidateRange(struct CachingProxy *, uint64_t, size_t);
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *, const void *);
static void interfaceDeinit(void *);
static void interfaceSetCallback(void *, void (*)(void *), void *);
static enum Result interfaceGetParam(void *, int, void *);
static enum Result interfaceSetParam(void *, int, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const CachingProxy =
    &(const struct InterfaceClass){
    .size = sizeof(struct CachingProxy),
    .init = interfaceInit,
    .deinit = interfaceDeinit,

    .setCallback = interfaceSetCallback,
    .getParam = interfaceGetParam,
    .setParam = interfaceSetParam,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
static struct CachedBlock *allocateBlock(struct CachingProxy *interface)
{
  struct CachedBlock *block = findEmptyBlock(interface);

  if (block == nullptr)
  {
    block = findOldestBlock(interface);
    assert(block != nullptr);

    if (block->flags & FLAG_DIRTY)
    {
      if (flushBlock(interface, block) != E_OK)
        return nullptr;
    }
  }

  return block;
}
/*----------------------------------------------------------------------------*/
static struct CachedBlock *findBlock(struct CachingProxy *interface,
    uint32_t address)
{
  for (size_t index = 0; index < pointerArraySize(&interface->pool); ++index)
  {
    struct CachedBlock * const block = *pointerArrayAt(&interface->pool, index);

    if (block->address == address)
      return block;
  }

  return nullptr;
}
/*----------------------------------------------------------------------------*/
static struct CachedBlock *findEmptyBlock(struct CachingProxy *interface)
{
  for (size_t index = 0; index < pointerArraySize(&interface->pool); ++index)
  {
    struct CachedBlock * const block = *pointerArrayAt(&interface->pool, index);

    if (block->address == BLOCK_RESERVED)
      return block;
  }

  return nullptr;
}
/*----------------------------------------------------------------------------*/
static struct CachedBlock *findOldestBlock(struct CachingProxy *interface)
{
  struct CachedBlock *result = nullptr;

  for (size_t index = 0; index < pointerArraySize(&interface->pool); ++index)
  {
    struct CachedBlock * const block = *pointerArrayAt(&interface->pool, index);

    if (block->address == BLOCK_RESERVED)
      continue;

    if (result == nullptr || block->timestamp < result->timestamp)
      result = block;
  }

  return result;
}
/*----------------------------------------------------------------------------*/
static enum Result flushAll(struct CachingProxy *interface)
{
  for (size_t index = 0; index < pointerArraySize(&interface->pool); ++index)
  {
    struct CachedBlock * const block = *pointerArrayAt(&interface->pool, index);

    if (block->flags & FLAG_DIRTY)
    {
      const enum Result res = flushBlock(interface, block);

      if (res != E_OK)
        return res;
    }
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result flushBlock(struct CachingProxy *interface,
    struct CachedBlock *block)
{
  const uint64_t position = (uint64_t)block->address * interface->blockSize;
  const enum Result res = ifSetParam(interface->pipe, IF_POSITION_64,
      &position);

  if (res != E_OK)
    return res;

  if (ifWrite(interface->pipe, block->data, interface->blockSize) !=
      interface->blockSize)
  {
    return E_DEVICE;
  }

  block->flags &= ~FLAG_DIRTY;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void invalidateRange(struct CachingProxy *interface, uint64_t address,
    size_t length)
{
  const uint64_t start = address / interface->blockSize;
  const uint64_t end = (address + length + interface->blockSize - 1)
      / interface->blockSize;

  for (size_t index = 0; index < pointerArraySize(&interface->pool); ++index)
  {
    struct CachedBlock * const block = *pointerArrayAt(&interface->pool, index);

    if (block->address != BLOCK_RESERVED
        && block->address >= start && block->address < end)
    {
      block->address = BLOCK_RESERVED;
      block->flags = 0;
      block->timestamp = 0;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *object, const void *configBase)
{
  const struct CachingProxyConfig * const config = configBase;
  assert(config != nullptr);
  assert(config->pipe != nullptr);
  assert(config->size > 0 && config->cache >= config->size);

  struct CachingProxy * const interface = object;

  interface->callback = nullptr;
  interface->callbackArgument = nullptr;
  interface->pipe = config->pipe;
  interface->arena = config->arena;
  interface->blockCount = config->cache / config->size;
  interface->blockSize = config->size;
  interface->address = 0;
  interface->timestamp = 0;
  interface->statistics.rMatch = 0;
  interface->statistics.rMiss = 0;
  interface->statistics.wMatch = 0;
  interface->statistics.wMiss = 0;

  if (!pointerArrayInit(&interface->pool, interface->blockCount))
    return E_MEMORY;

  const size_t bufferLength = interface->blockCount
      * (sizeof(struct CachedBlock) + interface->blockSize);

  interface->buffer = interface->arena != nullptr ?
      interface->arena : malloc(bufferLength);

  if (interface->buffer == nullptr)
  {
    pointerArrayDeinit(&interface->pool);
    return E_MEMORY;
  }

  struct CachedBlock *descriptor = interface->buffer;
  uint8_t *data = (uint8_t *)interface->buffer
      + interface->blockCount * sizeof(struct CachedBlock);

  for (size_t index = 0; index < interface->blockCount; ++index)
  {
    descriptor->address = BLOCK_RESERVED;
    descriptor->flags = 0;
    descriptor->timestamp = 0;
    descriptor->data = data;

    pointerArrayPushBack(&interface->pool, descriptor);

    ++descriptor;
    data += interface->blockSize;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interfaceDeinit(void *object)
{
  struct CachingProxy * const interface = object;

  if (interface->arena == nullptr)
    free(interface->buffer);

  pointerArrayDeinit(&interface->pool);
}
/*----------------------------------------------------------------------------*/
static void interfaceSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct CachingProxy * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;

  ifSetCallback(interface->pipe, callback, argument);
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceGetParam(void *object, int parameter, void *data)
{
  struct CachingProxy * const interface = object;

  switch ((enum CachingProxyParameter)parameter)
  {
    case IF_CACHE_READ_MATCH:
      *(uint32_t *)data = interface->statistics.rMatch;
      return E_OK;

    case IF_CACHE_READ_MISS:
      *(uint32_t *)data = interface->statistics.rMiss;
      return E_OK;

    case IF_CACHE_WRITE_MATCH:
      *(uint32_t *)data = interface->statistics.wMatch;
      return E_OK;

    case IF_CACHE_WRITE_MISS:
      *(uint32_t *)data = interface->statistics.wMiss;
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
  struct CachingProxy * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION_64:
    {
      const uint64_t position = *(const uint64_t *)data;
      const uint64_t block = position / interface->blockSize;

      if (block > UINT32_MAX)
        return E_VALUE;

      interface->address = (uint32_t)block;
      break;
    }

    case IF_POSITION:
    {
      const uint32_t position = *(const uint32_t *)data;

      interface->address = position / (uint32_t)interface->blockSize;
      break;
    }

    case IF_FLUSH:
    {
      const enum Result res = flushAll(interface);

      if (res != E_OK)
        return res;

      /* Restore the position after the writeback transfers */
      const uint64_t position = (uint64_t)interface->address
          * interface->blockSize;

      if (ifSetParam(interface->pipe, IF_POSITION_64, &position) != E_OK)
        return E_DEVICE;

      /* Pass control to the underlying interface */
      break;
    }

    default:
      break;
  }

  return ifSetParam(interface->pipe, parameter, data);
}
/*----------------------------------------------------------------------------*/
static size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct CachingProxy * const interface = object;

  if (length == interface->blockSize)
  {
    struct CachedBlock *block = findBlock(interface, interface->address);

    if (block == nullptr)
    {
      ++interface->statistics.rMiss;
      block = allocateBlock(interface);

      if (block == nullptr)
        return 0;

      block->flags = 0;
      block->timestamp = ++interface->timestamp;
      block->address = interface->address;

      const uint64_t position = (uint64_t)interface->address
          * interface->blockSize;
      const enum Result res =
          ifSetParam(interface->pipe, IF_POSITION_64, &position);

      if (res != E_OK)
      {
        block->address = BLOCK_RESERVED;
        return 0;
      }

      if (ifRead(interface->pipe, block->data, interface->blockSize) !=
          interface->blockSize)
      {
        block->address = BLOCK_RESERVED;
        return 0;
      }
    }
    else
    {
      ++interface->statistics.rMatch;
      block->timestamp = ++interface->timestamp;
    }

    memcpy(buffer, block->data, interface->blockSize);
    return interface->blockSize;
  }
  else
  {
    const uint64_t position = (uint64_t)interface->address
        * interface->blockSize;

    invalidateRange(interface, position, length);
    return ifRead(interface->pipe, buffer, length);
  }
}
/*----------------------------------------------------------------------------*/
static size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct CachingProxy * const interface = object;

  if (length == interface->blockSize)
  {
    struct CachedBlock *block = findBlock(interface, interface->address);

    if (block == nullptr)
    {
      ++interface->statistics.wMiss;
      block = allocateBlock(interface);

      if (block == nullptr)
        return 0;
    }
    else
    {
      ++interface->statistics.wMatch;
    }

    block->address = interface->address;
    block->flags = FLAG_DIRTY;
    block->timestamp = ++interface->timestamp;

    memcpy(block->data, buffer, interface->blockSize);
    return interface->blockSize;
  }
  else
  {
    const uint64_t position = (uint64_t)interface->address
        * interface->blockSize;

    invalidateRange(interface, position, length);
    return ifWrite(interface->pipe, buffer, length);
  }
}
