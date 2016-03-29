/*
 * platform/nxp/wakeup_interrupt.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_WAKEUP_INTERRUPT_H_
#define HALM_PLATFORM_NXP_WAKEUP_INTERRUPT_H_
/*----------------------------------------------------------------------------*/
#include <interrupt.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const WakeupInterrupt;
/*----------------------------------------------------------------------------*/
struct WakeupInterruptConfig
{
  /** Mandatory: pin used as interrupt source. */
  pinNumber pin;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Mandatory: external interrupt mode. */
  enum pinEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum pinPull pull;
};
/*----------------------------------------------------------------------------*/
struct WakeupInterrupt
{
  struct Entity base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Descriptor of the input pin used as interrupt source */
  union PinData pin;
  /* Edge sensitivity mode */
  enum pinEvent event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_WAKEUP_INTERRUPT_H_ */
