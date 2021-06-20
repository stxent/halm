/*
 * halm/platform/stm32/exti_base.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_EXTI_BASE_H_
#define HALM_PLATFORM_STM32_EXTI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/exti_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const ExtiBase;

struct ExtiBaseConfig
{
  /** Optional: pin used as interrupt source. */
  PinNumber pin;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: event number. */
  enum ExtiEvent channel;
};

struct ExtiBase
{
  struct Entity base;

  void (*handler)(void *);
  enum ExtiEvent channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_EXTI_BASE_H_ */
