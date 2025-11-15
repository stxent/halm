/*
 * halm/platform/lpc/gen_2/adc.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_H_
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
  /** Mandatory: trigger to start the conversion. */
  enum AdcEvent event;
  /** Mandatory: converter sequence identifier. */
  enum AdcSequence sequence;
  /** Optional: input sensitivity. */
  enum InputEvent sensitivity;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Optional: enable sequence preemption, available for sequence A only. */
  bool preemption;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
  /** Optional: use one trigger for one conversion. */
  bool singlestep;
};

struct AdcChannelTuple
{
  uint16_t data;
  struct AdcPin pin;
};

struct Adc
{
  struct AdcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Sequence control register value */
  uint32_t control;
  /* Interrupt priority */
  IrqPriority priority;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_H_ */
