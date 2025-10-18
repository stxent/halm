/*
 * sdma_memcopy.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/dma_memcopy.h>
#include <halm/platform/lpc/sdma_oneshot.h>
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
  static const struct SdmaSettings settings = {
      .burst = DMA_BURST_4,
      .width = DMA_WIDTH_WORD,
      .source = {
          .stride = SDMA_STRIDE_1,
          .wrap = false
      },
      .destination = {
          .stride = SDMA_STRIDE_1,
          .wrap = false
      }
  };
  const struct SdmaOneShotConfig config = {
      .request = SDMA_REQUEST_NONE,
      .trigger = SDMA_TRIGGER_NONE,
      .channel = channel,
      .priority = 0,
      .polarity = false
  };

  handler->dma = init(SdmaOneShot, &config);
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

  if (dmaStatus(handler->dma) == E_BUSY)
    return E_BUSY;

  handler->argument = argument;
  handler->callback = callback;

  dmaSetCallback(handler->dma, interruptHandler, handler);
  dmaAppend(handler->dma, destination, source, length);

  const enum Result res = dmaEnable(handler->dma);

  if (res != E_OK)
    dmaClear(handler->dma);

  return res;
}
