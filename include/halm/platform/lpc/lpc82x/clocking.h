/*
 * halm/platform/lpc/lpc82x/clocking.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for LPC82X series.
 */

#ifndef HALM_PLATFORM_LPC_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_CLOCKING_H_
#define HALM_PLATFORM_LPC_LPC82X_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockBranch
{
  CLOCK_BRANCH_MAIN,
  CLOCK_BRANCH_OUTPUT
};

enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};

enum [[gnu::packed]] WdtFrequency
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
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source.
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
/* May be initialized with the null pointer */
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
  enum ClockSource source;
  /**
   * Mandatory: PLL output divisor. The output divisor may be set
   * to divide by 2, 4, 8, 16.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier, result should be in the range from
   * 156 MHz to 320 MHz. Multiplier range is 1 to 32. Note that the input
   * frequency range is 10 to 25 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   */
};

/* Requires a PllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Optional: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: output pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Requires a ClockOutputConfig structure */
extern const struct ClockClass * const ClockOutput;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Optional: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Requires a GenericClockConfig structure */
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_CLOCKING_H_ */
