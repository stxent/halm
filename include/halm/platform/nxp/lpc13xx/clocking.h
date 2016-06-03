/*
 * halm/platform/nxp/lpc13xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for LPC13xx series.
 */

#ifndef HALM_PLATFORM_NXP_LPC13XX_CLOCKING_H_
#define HALM_PLATFORM_NXP_LPC13XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum clockBranch
{
  CLOCK_BRANCH_MAIN,
  CLOCK_BRANCH_OUTPUT,
  CLOCK_BRANCH_USB,
  CLOCK_BRANCH_WDT
};
/*----------------------------------------------------------------------------*/
enum clockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_USB_PLL,
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
  struct ClockClass base;

  enum clockBranch branch;
};
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const ExternalOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const WdtOsc;
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
extern const struct CommonClockClass * const ClockOutput;
extern const struct CommonClockClass * const MainClock;
extern const struct CommonClockClass * const UsbClock;
extern const struct CommonClockClass * const WdtClock;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Mandatory: clock source. */
  enum clockSource source;
  /** Mandatory: output pin. */
  pinNumber pin;
  /** Optional: input clock divisor in the range of 1 to 255. */
  uint8_t divisor;
};
/*----------------------------------------------------------------------------*/
struct CommonClockConfig
{
  /** Mandatory: clock source. */
  enum clockSource source;
  /** Optional: input clock divisor in the range of 1 to 255. */
  uint8_t divisor;
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
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   * @n Available options for USB PLL:
   *   - @b CLOCK_EXTERNAL.
   */
  enum clockSource source;
  /**
   * Mandatory: input clock multiplier, result should be in the range of
   * 156 MHz to 320 MHz. Multiplier range is 1 to 32. Note that the input
   * frequency range is 10 to 25 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: PLL output divisor. The output divisor may be set
   * to divide by 2, 4, 8, 16.
   */
  uint8_t divisor;
};
/*----------------------------------------------------------------------------*/
struct WdtOscConfig
{
  /** Optional: oscillator frequency. */
  enum wdtFrequency frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13XX_CLOCKING_H_ */
