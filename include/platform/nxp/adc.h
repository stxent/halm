/*
 * platform/nxp/adc.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ADC_H_
#define ADC_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "adc_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Adc;
/*----------------------------------------------------------------------------*/
/* TODO ADC: add sample rate configuration */
struct AdcConfig
{
  gpio_t pin; /* Mandatory: analog input */
  uint8_t precision; /* Optional: result width in bits */
};
/*----------------------------------------------------------------------------*/
struct Adc
{
  struct AdcBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to the input buffer */
  uint8_t *buffer;
  /* Samples left for conversion */
  volatile uint32_t left;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* ADC_H_ */
