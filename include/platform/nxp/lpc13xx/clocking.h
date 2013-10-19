/*
 * platform/nxp/lpc13xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CLOCKING_H_
#define CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <clock.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
extern uint32_t sysCoreClock;
/*----------------------------------------------------------------------------*/
extern const struct ClockClass *ExternalOsc;
extern const struct ClockClass *InternalOsc;
extern const struct ClockClass *SystemPll;
extern const struct ClockClass *UsbPll;
extern const struct ClockClass *MainClock;
extern const struct ClockClass *UsbClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  uint32_t frequency;
  bool bypass;
};
/*----------------------------------------------------------------------------*/
enum clockSource
{
  CLOCK_INTERNAL = 0,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_USB_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
struct MainClockConfig
{
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  uint16_t multiplier;
  uint8_t divider;
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
#endif /* CLOCKING_H_ */
