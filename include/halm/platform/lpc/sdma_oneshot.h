/*
 * halm/platform/lpc/sdma_oneshot.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMA_ONESHOT_H_
#define HALM_PLATFORM_LPC_SDMA_ONESHOT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/sdma_base.h>
/*----------------------------------------------------------------------------*/
extern const struct DmaClass * const SdmaOneShot;

struct SdmaOneShotConfig
{
  /** Mandatory: request type. */
  enum SdmaRequest request;
  /** Mandatory: request type. */
  enum SdmaTrigger trigger;
  /** Mandatory: channel number. */
  uint8_t channel;
  /** Optional: channel priority. */
  uint8_t priority;
  /** Optional: trigger polarity. */
  bool polarity;
};

struct SdmaOneShot
{
  struct SdmaBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* The destination address of the data to be transferred */
  uintptr_t destination;
  /* The source address of the data */
  uintptr_t source;
  /* Transfer configuration register value */
  uint32_t transferConfig;

  /* State of the transfer */
  uint8_t state;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMA_ONESHOT_H_ */
