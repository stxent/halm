/*
 * halm/generic/dma_memcopy.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_DMA_MEMCOPY_H_
#define HALM_GENERIC_DMA_MEMCOPY_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
struct DmaMemCopyHandler
{
  struct Dma *dma;

  void (*callback)(void *, enum Result);
  void *argument;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result dmaMemCopyInit(struct DmaMemCopyHandler *, uint8_t);
enum Result dmaMemCopyStart(struct DmaMemCopyHandler *, void *, const void *,
    size_t, void (*)(void *, enum Result), void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_DMA_MEMCOPY_H_ */
