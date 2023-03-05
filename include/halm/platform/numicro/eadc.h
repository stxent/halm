/*
 * halm/platform/numicro/eadc.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_EADC_H_
#define HALM_PLATFORM_NUMICRO_EADC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/eadc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Eadc;

struct EadcConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Optional: trigger delay in ADC clocks. */
  uint16_t offset;
  /** Mandatory: trigger to start the conversion. */
  enum AdcEvent event;
  /** Optional: external pin sensitivity. */
  enum PinEvent sensitivity;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: sampling time extension in ADC clocks. */
  uint8_t delay;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct Eadc
{
  struct EadcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Enabled channels mask */
  uint32_t enabled;
  /* Sampling Module settings */
  uint32_t sampling;
  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Interrupt priority */
  IrqPriority priority;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_EADC_H_ */
