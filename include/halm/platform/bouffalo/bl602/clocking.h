/*
 * halm/platform/bouffalo/bl602/clocking.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for i.MX RT106x series.
 */

#ifndef HALM_PLATFORM_BOUFFALO_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_H_
#define HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL_48MHZ,
  CLOCK_PLL_80MHZ,
  CLOCK_PLL_96MHZ,
  CLOCK_PLL_120MHZ,
  CLOCK_PLL_160MHZ,
  CLOCK_PLL_192MHZ,
  CLOCK_SYSTEM
};
/*----------------------------------------------------------------------------*/
struct DividedClockConfig
{
  /**
   * Mandatory: input clock divisor.
   *
   * This field defines the division factor applied to the input clock for
   * various peripheral and system clocks. The valid range depends
   * on the target clock domain:
   * - @b SOC clock: divisor must be in the range of 1 to 256 (inclusive).
   * - @b I2C clock: divisor must be in the range of 1 to 256 (inclusive).
   * - @b SPI clock: divisor must be in the range of 1 to 32 (inclusive).
   */
  uint16_t divisor;
};

/* Require an DividedClockConfig structure */
extern const struct ClockClass * const I2CClock;
extern const struct ClockClass * const SocClock;
extern const struct ClockClass * const SpiClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the crystal oscillator or external
   * clock source in Hz.
   *
   * Allowed values:
   * - 24 MHz   -> 24'000'000 Hz
   * - 32 MHz   -> 32'000'000 Hz
   * - 38.4 MHz -> 38'400'000 Hz
   * - 40 MHz   -> 40'000'000 Hz
   */
  uint32_t frequency;
};

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /**
   * Optional: input clock divisor.
   *
   * This field defines the division factor applied to the input clock before
   * generating the output clock signal. The valid divisor range is
   * from 1 to 255 (inclusive). A divisor of 1 results in no division.
   */
  uint16_t divisor;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input reference clock used to generate the
   * output clock signal. The set of available sources may vary depending
   * on the device capabilities.
   */
  enum ClockSource source;
};

/* Require a GenericClockConfig structure */
extern const struct ClockClass * const FlashClock;
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const UartClock;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Requires a PllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_CLOCKING_H_ */
