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
   * @n The divider value for SOC clock should be from 1 to 256.
   * @n The divider value for I2C clock should be from 1 to 256.
   * @n The divider value for SPI clock should be from 1 to 32.
   */
  uint16_t divisor;
};

/* Requires an DividedClockConfig structure */
extern const struct ClockClass * const I2CClock;
extern const struct ClockClass * const SocClock;
extern const struct ClockClass * const SpiClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source.
   */
  uint32_t frequency;
};

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Optional: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Requires a GenericClockConfig structure */
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
