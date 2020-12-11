/*
 * halm/platform/lpc/dac.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_DAC_H_
#define HALM_PLATFORM_LPC_DAC_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/dac_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Dac;

struct DacConfig
{
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  PinNumber pin;
};

struct Dac
{
  struct DacBase base;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_DAC_H_ */
