/*
 * platform/nxp/pin_interrupt.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_PIN_INTERRUPT_H_
#define PLATFORM_NXP_PIN_INTERRUPT_H_
/*----------------------------------------------------------------------------*/
#include <interrupt.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const PinInterrupt;
/*----------------------------------------------------------------------------*/
struct PinInterruptConfig
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
struct PinInterrupt
{
  struct Entity parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Descriptor of the input pin used as interrupt source */
  union PinData pin;
  /* Edge sensitivity mode */
  enum pinEvent event;
  /* Interrupt channel identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_PIN_INTERRUPT_H_ */
