/*
 * halm/platform/lpc/lpc17xx/clocking.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for LPC175x and LPC176x series.
 */

#ifndef HALM_PLATFORM_LPC_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC17XX_CLOCKING_H_
#define HALM_PLATFORM_LPC_LPC17XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_RTC,
  CLOCK_USB_PLL,
  CLOCK_MAIN
} __attribute__((packed));
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
extern const struct ClockClass * const RtcOsc;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: PLL output divisor.
   * @n System PLL accepts values in the range of 1 to 32.
   * @n USB PLL accepts a limited set of values: 2, 4, 8, 16.
   */
  uint16_t divisor;
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
   * Mandatory: clock source.
   * @n Available options for System PLL (PLL0):
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_RTC.
   * @n Available options for USB PLL (PLL1):
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Optional: input clock divisor in the range of 1 to 16. */
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
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Require a GenericClockConfig structure */
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const UsbClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_CLOCKING_H_ */
