/*
 * halm/platform/nxp/lpc13uxx/pin.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13UXX_PIN_H_
#define HALM_PLATFORM_NXP_LPC13UXX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0,
  PORT_1,
  PORT_USB
};

enum
{
  PIN_USB_DM,
  PIN_USB_DP
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_PIN/pin.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13UXX_PIN_H_ */
