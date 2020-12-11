/*
 * halm/platform/lpc/gen_2/pin_int.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PIN_INT_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_PIN_INT_H_
#define HALM_PLATFORM_LPC_GEN_2_PIN_INT_H_
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
  /** Mandatory: external interrupt mode. */
  enum PinEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum PinPull pull;
};

struct PinInt
{
  struct Interrupt base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Channel mask */
  uint16_t mask;
  /* Peripheral identifier */
  uint8_t channel;
  /* Pin event configuration */
  uint8_t event;
  /* Is interrupt enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_PIN_INT_H_ */
