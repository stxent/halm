/*
 * halm/platform/lpc/dac.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_DAC_H_
#define HALM_PLATFORM_LPC_DAC_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_DAC/dac_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
