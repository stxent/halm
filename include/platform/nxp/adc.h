/*
 * platform/nxp/adc.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_ADC_H_
#define PLATFORM_NXP_ADC_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <interface.h>
#include <spinlock.h>
#include "adc_base.h"
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const AdcUnit;
extern const struct InterfaceClass * const Adc;
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

  /* Current channel descriptor */
  struct Adc *current;
  /* Pointer to an interrupt handler */
  void (*handler)(void *);
  /* Access to converter block */
  spinlock_t lock;
};
/*----------------------------------------------------------------------------*/
struct AdcConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Mandatory: analog input. */
  pin_t pin;
  /** Optional: trigger to start the conversion. */
  enum adcEvent event;
};
/*----------------------------------------------------------------------------*/
struct Adc
{
  struct Entity parent;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *buffer;
  /* Number of samples to be converted */
  uint32_t left;
  /* Pin descriptor */
  struct AdcPin pin;
  /* Hardware trigger event */
  uint8_t event;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_ADC_H_ */
