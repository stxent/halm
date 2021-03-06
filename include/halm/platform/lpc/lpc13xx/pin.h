/*
 * halm/platform/lpc/lpc13xx/pin.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_PIN_H_
#define HALM_PLATFORM_LPC_LPC13XX_PIN_H_
/*----------------------------------------------------------------------------*/
enum
{
  PORT_0,
  PORT_1,
  PORT_2,
  PORT_3,
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
#endif /* HALM_PLATFORM_LPC_LPC13XX_PIN_H_ */
