/*
 * halm/platform/lpc/lpc17xx/pin_int.h
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PIN_INT_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC17XX_PIN_INT_H_
#define HALM_PLATFORM_LPC_LPC17XX_PIN_INT_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const PinInt;

struct PinIntConfig
{
  /** Mandatory: pin used as interrupt source. */
  PinNumber pin;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: external interrupt trigger. */
  enum InputEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum PinPull pull;
};

struct PinInt
{
  struct Interrupt base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pin mask of the input pin */
  uint32_t mask;
  /* Peripheral identifier */
  uint8_t channel;
  /* Edge sensitivity mode */
  uint8_t event;
  /* Is interrupt enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_PIN_INT_H_ */
