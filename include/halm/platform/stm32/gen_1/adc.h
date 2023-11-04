/*
 * halm/platform/stm32/gen_1/adc.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_1_ADC_H_
#define HALM_PLATFORM_STM32_GEN_1_ADC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Adc;

struct AdcConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: trigger to start the conversion, injected group is used. */
  enum AdcInjectedEvent event;
  /** Optional: trigger sensitivity when external trigger is used. */
  enum InputEvent sensitivity;
  /** Optional: sampling time of a single channel. */
  enum AdcSamplingTime time;
  /** Optional: resolution of the conveserion. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct Adc
{
  struct AdcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Precalculated value ot the Control Register 1 */
  uint32_t control1;
  /* Precalculated value ot the Control Register 2 */
  uint32_t control2;
  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Sampling time */
  enum AdcSamplingTime time;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_ADC_H_ */
