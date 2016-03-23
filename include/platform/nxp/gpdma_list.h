/*
 * platform/nxp/gpdma_list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPDMA_LIST_H_
#define PLATFORM_NXP_GPDMA_LIST_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <platform/nxp/gpdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const GpDmaList;
/*----------------------------------------------------------------------------*/
struct GpDmaListConfig
{
  /** Mandatory: number of blocks in the chain. */
  uint16_t number;
  /** Mandatory: size in elements of each block. */
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
  /** Mandatory: request connection to the peripheral or memory. */
  enum gpDmaEvent event;
  /** Mandatory: transfer direction. */
  enum gpDmaType type;
  /** Mandatory: width of source and destination transfers. */
  enum dmaWidth width;
  /**
   * Optional: set @b true to reduce channel interrupts count by disabling
   * requests for intermediate states of the transfer.
   */
  bool silent;
};
/*----------------------------------------------------------------------------*/
struct GpDmaListRuntimeConfig
{
  /** Mandatory: destination configuration. */
  struct {
    bool increment;
  } destination;
  /** Mandatory: source configuration. */
  struct {
    bool increment;
  } source;
};
/*----------------------------------------------------------------------------*/
struct GpDmaListEntry
{
  uint32_t source;
  uint32_t destination;
  uint32_t next;
  uint32_t control;
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
struct GpDmaList
{
  struct GpDmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Descriptor list container */
  struct GpDmaListEntry *list;

  /* Capacity of the chain in blocks */
  uint16_t capacity;
  /* Current block in the chain */
  uint16_t current;
  /* Maximum size of each block */
  uint16_t size;
  /* Number of queued buffers */
  uint16_t queued;
  /* Width in bytes of each element */
  uint8_t width;

  /* Reduce interrupts count */
  bool silent;
  /* Last transfer status */
  bool error;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPDMA_LIST_H_ */
