/*
 * platform/nxp/dac.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_DAC_H_
#define PLATFORM_NXP_DAC_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_DAC/dac_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
#endif /* PLATFORM_NXP_DAC_H_ */
