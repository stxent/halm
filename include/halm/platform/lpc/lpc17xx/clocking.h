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
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_RTC,
  CLOCK_USB_PLL,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the crystal oscillator or external clock source.
   *
   * This field specifies the operating frequency of the external clock input,
   * which may be either a crystal oscillator or an external clock signal.
   * The supported frequency range is strictly limited to 1 MHz to 25 MHz
   * (inclusive).
   */
  uint32_t frequency;

  /**
   * Optional: enable bypass mode.
   *
   * When enabled, this flag configures the oscillator circuit to bypass the
   * internal crystal oscillator path and directly accept an external clock
   * signal as the reference source.
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
   *
   * This field defines the division factor applied to the PLL output frequency.
   * The valid range depends on the PLL type:
   * - **System PLL**: the divisor must be in the range of 1 to 32 (inclusive).
   * - **USB PLL**: only specific even values are supported: 2, 4, 8 and 16.
   */
  uint16_t divisor;

  /**
   * Mandatory: input clock multiplier.
   *
   * This field specifies the multiplication factor applied to the input clock
   * to achieve the desired PLL operating frequency. The effective frequency
   * range and constraints vary by PLL type:
   * - **System PLL**: operates in the range from 275 MHz to 550 MHz.
   *   The multiplier range is 6 to 512. The input frequency range is
   *   32 kHz to 50 MHz. When a 32 kHz clock is used, an additional set of
   *   values is available.
   * - **USB PLL**: operates within the range of 156 MHz to 320 MHz.
   *   The input clock frequency must be within the range of 10 MHz to 25 MHz.
   *
   * @note The actual output frequency is calculated as:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input clock signal for the PLL.
   * Available options depend on the PLL instance:
   * - For **System PLL** (PLL0):
   *   - @b CLOCK_INTERNAL
   *   - @b CLOCK_EXTERNAL
   *   - @b CLOCK_RTC
   * - For **USB PLL** (PLL1):
   *   - Only @b CLOCK_EXTERNAL is supported.
   */
  enum ClockSource source;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Optional: input clock divisor in the range from 1 to 16. */
  uint16_t divisor;
  /** Mandatory: output pin. */
  PinNumber pin;
  /** Mandatory: clock source selection. */
  enum ClockSource source;
};

/* Requires a ClockOutputConfig structure */
extern const struct ClockClass * const ClockOutput;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Mandatory: clock source selection. */
  enum ClockSource source;
};

/* Require a GenericClockConfig structure */
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const UsbClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC17XX_CLOCKING_H_ */
