/*
 * halm/platform/stm32/stm32f0xx/clocking.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for STM32F0xx series.
 */

#ifndef HALM_PLATFORM_STM32_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_H_
#define HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_INTERNAL_14,
  CLOCK_INTERNAL_48,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_SYSTEM
};
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source. The input frequency range is 4 to 32 MHz.
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
extern const struct ClockClass * const InternalOsc48;
/*----------------------------------------------------------------------------*/
struct SystemPllConfig
{
  /**
   * Mandatory: clock source.
   * @n Available options:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_INTERNAL_48.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
  /** Mandatory: PLL input divisor. Divisor range is 1 to 16. */
  uint16_t divisor;
  /** Mandatory: PLL multiplier. The multiplier range is 2 to 16. */
  uint16_t multiplier;
};

/* Requires a SystemPllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct SystemClockConfig
{
  /**
   * Mandatory: system clock source.
   * @n Available sources are:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_INTERNAL_48.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_PLL.
   */
  enum ClockSource source;
};

/* Requires a SystemClockConfig structure */
extern const struct ClockClass * const SystemClock;
/*----------------------------------------------------------------------------*/
struct BusClockConfig
{
  /**
   * Mandatory: bus clock divisor.
   * @n Available options for AHB: 2, 4, 8, 16, 64, 128, 256, 512.
   * @n Available options for APB: 2, 4, 8, 16.
   */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const ApbClock;
//extern const struct ClockClass * const AdcClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_H_ */
