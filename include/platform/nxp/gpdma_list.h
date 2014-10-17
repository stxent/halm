/*
 * platform/nxp/gpdma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPDMA_LIST_H_
#define PLATFORM_NXP_GPDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include "gpdma_base.h"
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDmaList;
/*----------------------------------------------------------------------------*/
struct GpDmaListConfig
{
  /** Mandatory: number of elements in the list. */
  uint16_t number;
  /** Mandatory: size of each element. */
  uint16_t size;
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
  /**
   * Optional: buffer organization selection for scatter-gather transfers.
   * Set @b true for circular buffer type or @b false for linear type.
   */
  bool circular;
  /**
   * Optional: set @b true to reduce channel interrupts count by disabling
   * requests for intermediate states of the transfer.
   */
  bool silence;
};
/*----------------------------------------------------------------------------*/
struct GpDmaListItem
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct GpDmaList
{
  struct GpDmaBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Descriptor list container */
  struct GpDmaListItem *buffer;

  /* List capacity */
  uint16_t capacity;
  /* Current list length */
  uint16_t number;
  /* Size of an element */
  uint16_t size;

  /* Circular buffer flag */
  bool circular;
  /* Channel error flag */
  bool error;
  /* Reduce interrupt count flag */
  bool silence;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPDMA_LIST_H_ */
