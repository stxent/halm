/*
 * halm/platform/stm32/exti.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_EXTI_H_
#define HALM_PLATFORM_STM32_EXTI_H_
/*----------------------------------------------------------------------------*/
#include <halm/interrupt.h>
#include <halm/irq.h>
#include <halm/platform/stm32/exti_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterruptClass * const Exti;

struct ExtiConfig
{
  /** Optional: pin used as interrupt source. */
  PinNumber pin;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: event number. */
  enum ExtiEvent channel;
  /** Optional: external interrupt sensitivity. */
  enum InputEvent event;
  /** Optional: enables pull-up or pull-down resistors for input pin. */
  enum PinPull pull;
};

struct Exti
{
  struct ExtiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Channel mask */
  uint32_t mask;
  /* Pin event configuration */
  uint8_t event;
  /* Is interrupt enabled */
  bool enabled;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_EXTI_H_ */
