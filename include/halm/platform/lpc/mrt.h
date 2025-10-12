/*
 * halm/platform/lpc/mrt.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_MRT_H_
#define HALM_PLATFORM_LPC_MRT_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/mrt_base.h>
/*----------------------------------------------------------------------------*/
extern const struct TimerClass * const Mrt;

struct MrtConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct Mrt
{
  struct MrtBase base;

  /* User interrupt handler */
  void (*callback)(void *);
  /* Argument passed to the user interrupt handler */
  void *callbackArgument;

  /* Time interval value */
  uint32_t interval;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_MRT_H_ */
