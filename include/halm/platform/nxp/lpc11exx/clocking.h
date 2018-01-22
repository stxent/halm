/*
 * halm/platform/nxp/lpc11exx/clocking.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for LPC11Exx series.
 */

#ifndef HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_H_
#define HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum ClockBranch
{
  CLOCK_BRANCH_MAIN,
  CLOCK_BRANCH_OUTPUT
};

enum ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};

enum WdtFrequency
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

struct CommonClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
};
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * external clock source.
   */
  uint32_t frequency;
  /**
   * Optional: enable bypass. Bypassing should be enabled when using
   * an external clock source instead of the crystal oscillator.
   */
  bool bypass;
};

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;
/*----------------------------------------------------------------------------*/
/* May be called with the null pointer */
extern const struct ClockClass * const InternalOsc;
/*----------------------------------------------------------------------------*/
struct WdtOscConfig
{
  /** Optional: oscillator frequency. */
  enum WdtFrequency frequency;
};

/* Requires a WdtOscConfig structure */
extern const struct ClockClass * const WdtOsc;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
  /**
   * Mandatory: PLL output divisor. The output divisor may be set
   * to divide by 2, 4, 8, 16.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier, result should be in the range of
   * 156 MHz to 320 MHz. Multiplier range is 1 to 32. Note that the input
   * frequency range is 10 to 25 MHz.
   */
  uint16_t multiplier;
};

/* Requires a PllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /** Optional: input clock divisor in the range of 1 to 255. */
  uint16_t divisor;
  /** Mandatory: output pin. */
  PinNumber pin;
};

/* Requires a ClockOutputConfig structure */
extern const struct CommonClockClass * const ClockOutput;
/*----------------------------------------------------------------------------*/
struct CommonClockConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /** Optional: input clock divisor in the range of 1 to 255. */
  uint16_t divisor;
};

/* Requires a CommonClockConfig structure */
extern const struct CommonClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC11EXX_CLOCKING_H_ */
