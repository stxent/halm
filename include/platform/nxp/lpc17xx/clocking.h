/*
 * platform/nxp/lpc17xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for LPC175x and LPC176x series.
 */

#ifndef PLATFORM_NXP_LPC17XX_CLOCKING_H_
#define PLATFORM_NXP_LPC17XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <clock.h>
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const ExternalOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const RtcOsc;
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const UsbClock;
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
enum clockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_RTC,
  CLOCK_USB_PLL,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL (PLL0):
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_RTC.
   * @n Available options for USB PLL (PLL1):
   *   - @b CLOCK_EXTERNAL.
   */
  enum clockSource source;
  /**
   * Mandatory: input clock multiplier.
   * @n Oscillator of the System PLL operates in the range of 275 MHz
   * to 550 MHz, multiplier range is 6 to 512. Input frequency range is
   * 32 kHz to 50 MHz. When 32 kHz clock is used, an additional set of values
   * is available.
   * @n Oscillator of the USB PLL operates in the range of 156 MHz to 320 MHz.
   * Input frequency range is 10 MHz to 25 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: PLL output divisor.
   * @n System PLL accepts values in the range of 1 to 32.
   * @n USB PLL accepts a limited set of values: 2, 4, 8, 16.
   */
  uint8_t divisor;
};
/*----------------------------------------------------------------------------*/
struct CommonClockConfig
{
  /** Mandatory: clock source. */
  enum clockSource source;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC17XX_CLOCKING_H_ */
