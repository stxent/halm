/*
 * platform/nxp/dac.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DAC_H_
#define DAC_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include "dac_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Dac;
/*----------------------------------------------------------------------------*/
struct DacConfig
{
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  pin_t pin;
};
/*----------------------------------------------------------------------------*/
struct Dac
{
  struct DacBase parent;
};
/*----------------------------------------------------------------------------*/
#endif /* DAC_H_ */
