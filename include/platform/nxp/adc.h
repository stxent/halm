/*
 * platform/nxp/adc.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_H_
#define ADC_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <interface.h>
#include <spinlock.h>
#include "adc_base.h"
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *AdcUnit;
extern const struct InterfaceClass *Adc;
/*----------------------------------------------------------------------------*/
struct AdcUnitConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: interrupt priority. */
  priority_t priority;
};
/*----------------------------------------------------------------------------*/
struct AdcUnit
{
  struct AdcUnitBase parent;

  /* Conversion channel currently in use */
  struct Adc *current;
  /* Access to converter block */
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
struct AdcConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Mandatory: analog input. */
  gpio_t pin;
  /** Optional: trigger to start the conversion. */
  enum adcEvent event;
};
/*----------------------------------------------------------------------------*/
struct Adc
{
  struct Interface parent;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *buffer;
  /* Number of samples to be converted */
  uint32_t left;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Peripheral channel */
  uint8_t channel;
  /* Hardware trigger event */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_H_ */
