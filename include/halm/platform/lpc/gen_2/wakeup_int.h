/*
 * halm/platform/lpc/gen_2/wakeup_int.h
 * Copyright (C) 2015, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_WAKEUP_INT_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_WAKEUP_INT_H_
#define HALM_PLATFORM_LPC_GEN_2_WAKEUP_INT_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const WakeupInt;

struct WakeupIntConfig
{
  /** Mandatory: pin used as interrupt source. */
  PinNumber pin;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: external interrupt mode. */
  enum PinEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum PinPull pull;
};

struct WakeupInt
{
  struct Entity base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Start register mask */
  uint32_t mask;
  /* Interrupt channel */
  uint8_t channel;
  /* Start register index */
  uint8_t index;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_WAKEUP_INT_H_ */
