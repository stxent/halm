/*
 * halm/platform/nxp/pin_interrupt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_PIN_INTERRUPT_H_
#define HALM_PLATFORM_NXP_PIN_INTERRUPT_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const PinInterrupt;
/*----------------------------------------------------------------------------*/
struct PinInterruptConfig
{
  /** Mandatory: pin used as interrupt source. */
  PinNumber pin;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: external interrupt mode. */
  enum pinEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum pinPull pull;
};
/*----------------------------------------------------------------------------*/
struct PinInterrupt
{
  struct Entity base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Descriptor of the input pin used as interrupt source */
  struct PinData pin;
  /* Interrupt channel identifier */
  uint8_t channel;
  /* Edge sensitivity mode */
  uint8_t event;
  /* Is interrupt enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_PIN_INTERRUPT_H_ */
