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
  uint8_t channel; /* Mandatory: peripheral number */
  priority_t priority; /* Optional: interrupt priority */
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
  struct AdcUnit *parent; /* Mandatory: peripheral unit */
  gpio_t pin; /* Mandatory: analog input */
  uint8_t event; /* Optional: hardware triggered conversion source */
};
/*----------------------------------------------------------------------------*/
struct Adc
{
  struct Interface parent;

  /* Pointer to parental unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to the input buffer */
  uint8_t *buffer;
  /* Number of samples to be converted */
  volatile uint32_t left;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Peripheral channel */
  uint8_t channel;
  /* Hardware trigger */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_H_ */
