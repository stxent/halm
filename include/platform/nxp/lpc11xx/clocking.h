/*
 * platform/nxp/lpc11xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC11XX_CLOCKING_H_
#define PLATFORM_NXP_LPC11XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <clock.h>
/*----------------------------------------------------------------------------*/
enum clockBranch
{
  CLOCK_BRANCH_MAIN,
  CLOCK_BRANCH_OUTPUT,
  CLOCK_BRANCH_WDT
};
/*----------------------------------------------------------------------------*/
enum clockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
enum wdtFrequency
{
  /* Watchdog oscillator analog output frequency in kHz */
  WDT_FREQ_600 = 0x01,
  WDT_FREQ_1050,
  WDT_FREQ_1400,
  WDT_FREQ_1750,
  WDT_FREQ_2100,
  WDT_FREQ_2400,
  WDT_FREQ_2700,
  WDT_FREQ_3000,
  WDT_FREQ_3250,
  WDT_FREQ_3500,
  WDT_FREQ_3750,
  WDT_FREQ_4000,
  WDT_FREQ_4200,
  WDT_FREQ_4400,
  WDT_FREQ_4600
};
/*----------------------------------------------------------------------------*/
struct CommonClockClass
{
  struct ClockClass parent;

  enum clockBranch branch;
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const ExternalOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const WdtOsc;
extern const struct ClockClass * const SystemPll;
extern const struct CommonClockClass * const ClockOutput;
extern const struct CommonClockClass * const MainClock;
extern const struct CommonClockClass * const WdtClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  uint32_t frequency;
  bool bypass;
};
/*----------------------------------------------------------------------------*/
struct WdtOscConfig
{
  enum wdtFrequency frequency;
};
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  uint16_t multiplier;
  uint8_t divisor;
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
struct CommonClockConfig
{
  uint8_t divisor;
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC11XX_CLOCKING_H_ */
