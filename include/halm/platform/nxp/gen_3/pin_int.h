/*
 * halm/platform/nxp/gen_3/pin_int.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_3_PIN_INT_H_
#define HALM_PLATFORM_NXP_GEN_3_PIN_INT_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/pin.h>
#include <halm/platform/nxp/gen_3/pin_int_base.h>
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
  struct PinIntBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Channel mask */
  uint8_t mask;
  /* Is interrupt enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_3_PIN_INT_H_ */
