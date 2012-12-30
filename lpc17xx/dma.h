/*
 * dma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 *  @file
 *  Abstract DMA interface for embedded systems: definitions.
 */

#ifndef DMA_H_
#define DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "error.h"
#include "entity.h"
/*----------------------------------------------------------------------------*/
/** DMA burst transfer size. */
enum dmaBurst
{
  DMA_BURST_1 = 0,
  DMA_BURST_2,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16,
  DMA_BURST_32,
  DMA_BURST_64,
  DMA_BURST_128,
  DMA_BURST_256,
  DMA_BURST_512,
  DMA_BURST_1024
};
/*----------------------------------------------------------------------------*/
/** DMA transfer width. */
enum dmaWidth
{
  DMA_WIDTH_BYTE = 0,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct DmaClass
{
  CLASS_GENERATOR

  enum result (*start)(void *, void *, const void *, uint32_t);
  enum result (*startList)(void *, const void *);
  void (*stop)(void *);
  void (*halt)(void *);
  void (*linkItem)(void *, void *, void *, void *, const void *, uint16_t);
  /* TODO Get the number of completed transfers */
  /* uint16_t getCount(const void *); */
};
/*----------------------------------------------------------------------------*/
struct Dma
{
  struct Entity parent;

  bool active; /* Transfer active flag */
  uint8_t channel; /* Channel may have different meaning */
};
/*----------------------------------------------------------------------------*/
/* Virtual functions */
enum result dmaStart(void *, void *, const void *, uint32_t);
enum result dmaStartList(void *, const void *);
void dmaStop(void *);
void dmaHalt(void *);
void dmaLinkItem(void *, void *, void *, void *, const void *, uint16_t);
/*----------------------------------------------------------------------------*/
/* Non-virtual functions */
bool dmaIsActive(const struct Dma *);
/*----------------------------------------------------------------------------*/
#endif /* DMA_H_ */
