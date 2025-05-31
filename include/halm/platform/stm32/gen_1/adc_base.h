/*
 * halm/platform/stm32/gen_1/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_1_ADC_BASE_H_
#define HALM_PLATFORM_STM32_GEN_1_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcBase;

struct AdcBaseConfig
{
  /** Mandatory: external event select for injected group. */
  enum AdcInjectedEvent injected;
  /** Mandatory: external event select for regular group. */
  enum AdcEvent regular;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);

  /* Unique peripheral identifier */
  uint8_t channel;
};

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_ADC_BASE_H_ */
