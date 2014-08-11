/*
 * platform/nxp/lpc17xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CLOCKING_H_
#define CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <clock.h>
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const ExternalOsc;
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  uint32_t frequency;
  bool bypass;
};
/*----------------------------------------------------------------------------*/
enum clockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_RTC,
  CLOCK_USB_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  uint16_t multiplier;
  uint8_t divider;
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
struct MainClockConfig
{
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
#endif /* CLOCKING_H_ */
