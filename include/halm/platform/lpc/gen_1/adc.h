/*
 * halm/platform/lpc/gen_1/adc.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_ADC_H_
#define HALM_PLATFORM_LPC_GEN_1_ADC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Adc;

struct AdcConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: number of bits of accuracy of the result. */
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

  /* Data registers used */
  const uint32_t *dr[8];
  /* Pin descriptors */
  struct AdcPin pins[8];
  /* Output buffer */
  uint16_t buffer[8];

  /* Interrupt priority */
  IrqPriority priority;
  /* Pin count */
  uint8_t count;
  /* Event channel */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_ADC_H_ */
