/*
 * clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef CLOCKING_H_
#define CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "entity.h"
#include "platform/clock.h"
/*----------------------------------------------------------------------------*/
extern const struct ClockClass *ExternalOsc;
extern const struct ClockClass *InternalOsc;
extern const struct ClockClass *MainClock;
//extern const struct ClockClass *SystemPll;
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
////TODO Calculate dividers from specified frequency
//struct SystemPllConfig
//{
//  uint8_t msel;
//  uint8_t psel;
//};
///*----------------------------------------------------------------------------*/
//struct SystemPll
//{
//  struct Clock parent;
//
//  uint8_t feedbackDivider, postDivider;
//};
/*----------------------------------------------------------------------------*/
#endif /* CLOCKING_H_ */
