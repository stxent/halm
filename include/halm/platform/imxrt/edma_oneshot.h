/*
 * halm/platform/imxrt/edma_oneshot.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_EDMA_ONESHOT_H_
#define HALM_PLATFORM_IMXRT_EDMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/imxrt/edma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const EdmaOneShot;

struct EdmaOneShotConfig
{
  /** Mandatory: request connection. */
  enum EdmaEvent event;
  /** Mandatory: channel priority. */
  enum EdmaPriority priority;
  /** Mandatory: channel number, from 0 to 15. */
  uint8_t channel;
};

struct EdmaOneShot
{
  struct EdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Destination address base */
  uintptr_t destination;
  /* Source address base */
  uintptr_t source;
  /* Number of transfers: total number of bytes in all loops */
  uint32_t count;
  /* Transfer attributes */
  uint16_t attributes;
  /* Burst size: number of bytes in the minor loop */
  uint16_t burst;
  /* Destination address offset */
  int16_t dstOffset;
  /* Source address offset */
  int16_t srcOffset;

  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_EDMA_ONESHOT_H_ */
