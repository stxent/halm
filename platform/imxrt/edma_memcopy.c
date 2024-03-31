/*
 * edma_memcopy.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/core/cortex/cache.h>
#include <halm/generic/dma_memcopy.h>
#include <halm/platform/imxrt/edma_oneshot.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct DmaMemCopyHandler * const handler = object;

  if (handler->callback != NULL)
    handler->callback(handler->argument, dmaStatus(handler->dma));
}
/*----------------------------------------------------------------------------*/
enum Result dmaMemCopyInit(struct DmaMemCopyHandler *handler, uint8_t channel)
{
  static const struct EdmaSettings settings = {
      .burst = 1,

      .source = {
          .offset = 4,
          .width = DMA_WIDTH_WORD
      },
      .destination = {
          .offset = 4,
          .width = DMA_WIDTH_WORD
      }
  };
  const struct EdmaOneShotConfig config = {
      .event = EDMA_MEMORY,
      .priority = DMA_PRIORITY_LOW,
      .channel = channel
  };

  handler->dma = init(EdmaOneShot, &config);
  if (handler->dma == NULL)
    return E_ERROR;

  dmaConfigure(handler->dma, &settings);
  handler->callback = NULL;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
enum Result dmaMemCopyStart(struct DmaMemCopyHandler *handler,
    void *destination, const void *source, size_t length,
    void (*callback)(void *, enum Result), void *argument)
{
  if (((uintptr_t)destination & 3) || ((uintptr_t)source & 3) || (length & 3))
  {
    /* Unaligned buffer addresses or length */
    return E_MEMORY;
  }

  enum Result res;

  if ((res = dmaStatus(handler->dma)) != E_OK)
    return res;

  handler->argument = argument;
  handler->callback = callback;

  dmaSetCallback(handler->dma, interruptHandler, handler);
  dmaAppend(handler->dma, destination, source, length);

  dCacheClean((uintptr_t)source, length);
  dCacheInvalidate((uintptr_t)destination, length);
  res = dmaEnable(handler->dma);

  if (res != E_OK)
    dmaClear(handler->dma);

  return res;
}
