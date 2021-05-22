/*
 * halm/platform/lpc/adc_bus.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BUS_H_
#define HALM_PLATFORM_LPC_ADC_BUS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcBus;

struct AdcBusConfig
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

struct AdcBus
{
  struct AdcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an output buffer */
  uint16_t *buffer;

  /* Pin descriptors */
  struct AdcPin pins[8];
  /* Interrupt priority */
  IrqPriority priority;
  /* Pin count */
  uint8_t count;
  /* Event channel */
  uint8_t event;
  /* Enable the blocking mode instead of the non-blocking zero-copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ADC_BUS_H_ */
