/*
 * halm/platform/stm32/dma.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DMA_H_
#define HALM_PLATFORM_STM32_DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#define DMA_MAX_TRANSFER_SIZE ((1 << 16) - 1)
/*----------------------------------------------------------------------------*/
/** Burst size. */
enum [[gnu::packed]] DmaBurst
{
  DMA_BURST_1,
  DMA_BURST_4,
  DMA_BURST_8,
  DMA_BURST_16
};

/** Transfer priority. */
enum [[gnu::packed]] DmaPriority
{
  DMA_PRIORITY_LOW,
  DMA_PRIORITY_MEDIUM,
  DMA_PRIORITY_HIGH,
  DMA_PRIORITY_VERY_HIGH
};

/** Transfer type. */
enum [[gnu::packed]] DmaType
{
  /** Peripheral to memory. */
  DMA_TYPE_P2M,
  /** Memory to peripheral. */
  DMA_TYPE_M2P,
  /** Memory to memory. */
  DMA_TYPE_M2M
};

/** Transfer width. */
enum [[gnu::packed]] DmaWidth
{
  DMA_WIDTH_BYTE,
  DMA_WIDTH_HALFWORD,
  DMA_WIDTH_WORD
};

struct DmaSettings
{
  /** Mandatory: source configuration. */
  struct
  {
    /** Mandatory: burst size for source transfers. */
    enum DmaBurst burst;
    /** Mandatory: source transfer width. */
    enum DmaWidth width;
    /** Mandatory: enable increment of the source address. */
    bool increment;
  } source;

  /** Mandatory: destination configuration. */
  struct
  {
    /** Mandatory: burst size for destination transfers. */
    enum DmaBurst burst;
    /** Mandatory: destination transfer width. */
    enum DmaWidth width;
    /** Mandatory: enable increment of the destination address. */
    bool increment;
  } destination;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DMA_H_ */
