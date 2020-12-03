/*
 * halm/platform/stm32/stm32f1xx/clocking.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for STM32F1xx series.
 */

#ifndef HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_
#define HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_PLL2,
  CLOCK_PLL3,
  CLOCK_SYSTEM
};
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source. The input frequency range for connectivity-line
   * devices is 3 to 25 MHz, the range for other devices is 4 to 16 MHz.
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
struct MainPllConfig
{
  /**
   * Mandatory: clock source.
   * @n Available options for connectivity-line devices:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_PLL2.
   * @n Available options for low-, medium-, high- and XL-density devices:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
  /**
   * Mandatory: PLL input divisor.
   * @n Available options for connectivity-line devices:
   *   - @b CLOCK_INTERNAL: division by 2.
   *   - @b CLOCK_EXTERNAL: divisor range is 1 to 16.
   * @n Available options for other devices:
   *   - @b CLOCK_INTERNAL: division by 2.
   *   - @b CLOCK_EXTERNAL: division by 1 or 2.
   */
  uint16_t divisor;
  /**
   * Mandatory: PLL multiplier. The multiplier range for connectivity-line
   * devices is 4 to 9, the range for other devices is 2 to 16.
   */
  uint16_t multiplier;
};

/* Requires a MainPllConfig structure */
extern const struct ClockClass * const MainPll;
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const Pll2;
extern const struct ClockClass * const Pll3;
/*----------------------------------------------------------------------------*/
struct SystemClockConfig
{
  /**
   * Mandatory: system clock source. Available sources are:
   * @b CLOCK_INTERNAL, @b CLOCK_EXTERNAL and @b CLOCK_PLL.
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
   * @n Available options for both APB: 2, 4, 8, 16.
   * @n Available options for ADC: 2, 4, 6, 8.
   */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const AdcClock;
extern const struct ClockClass * const Apb1Clock;
extern const struct ClockClass * const Apb2Clock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_ */
