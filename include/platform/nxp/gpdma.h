/*
 * platform/nxp/gpdma.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef GPDMA_H_
#define GPDMA_H_
/*----------------------------------------------------------------------------*/
#include "gpdma_base.h"
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDma;
/*----------------------------------------------------------------------------*/
struct GpDmaConfig
{
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Mandatory: destination configuration. */
  struct {
    bool increment;
  } destination;
  /** Mandatory: source configuration. */
  struct {
    bool increment;
  } source;
  /** Mandatory: number of transfers that make up a burst transfer request. */
  enum dmaBurst burst;
  /** Mandatory: source and destination transfer widths. */
  enum dmaWidth width;
  /** Mandatory: request connection to the peripheral or memory. */
  enum gpDmaEvent event;
  /** Mandatory: transfer type. */
  enum gpDmaType type;
};
/*----------------------------------------------------------------------------*/
struct GpDma
{
  struct GpDmaBase parent;

  void (*callback)(void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
#endif /* GPDMA_H_ */
