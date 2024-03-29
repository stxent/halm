/*
 * halm/platform/lpc/bod.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_BOD_H_
#define HALM_PLATFORM_LPC_BOD_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/bod.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const Bod;

struct BodConfig
{
  /**
   * Mandatory: if the voltage falls below event level, an interrupt signal
   * will be asserted.
   */
  enum BodEventLevel eventLevel;
  /**
   * Mandatory: if the voltage falls below reset level, a reset signal
   * will be asserted to deactivate the MCU.
   */
  enum BodResetLevel resetLevel;
  /** Optional: interrupt priority. */
  IrqPriority priority;
};

struct Bod
{
  struct Interrupt base;

  void (*callback)(void *);
  void *callbackArgument;

  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_BOD_H_ */
