/*
 * halm/platform/stm32/stm32f1xx/clocking.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for STM32F1xx series.
 */

#ifndef HALM_PLATFORM_STM32_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_
#define HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
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
   * Mandatory: frequency of the crystal oscillator or external clock source.
   *
   * This field specifies the operating frequency of the external clock input,
   * which may be either a crystal oscillator or an external clock signal.
   * The supported frequency range depends on the device type:
   * - Connectivity-line devices: 3 MHz to 25 MHz.
   * - Other devices (low-, medium-, high- and XL-density): 4 MHz to 16 MHz.
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
extern const struct ClockClass * const InternalLowSpeedOsc;
extern const struct ClockClass * const InternalOsc;
/*----------------------------------------------------------------------------*/
struct MainPllConfig
{
  /**
   * Mandatory: PLL input divisor.
   *
   * This field defines the division factor applied to the input clock before
   * it is fed into the PLL. The valid divisor values depend on the device type:
   * - For connectivity-line devices:
   *   - @b CLOCK_INTERNAL: the input clock is automatically divided by 2.
   *   - @b CLOCK_EXTERNAL: the divisor may be set to any integer value
   *     in the range of 1 to 16 (inclusive).
   * - For other devices (low-, medium-, high- and XL-density):
   *   - @b CLOCK_INTERNAL: the input clock is automatically divided by 2.
   *   - @b CLOCK_EXTERNAL: the divisor is restricted to either 1 or 2.
   */
  uint16_t divisor;

  /**
   * Mandatory: PLL multiplier.
   *
   * This field specifies the multiplication factor applied to the pre-divided
   * input clock to achieve the desired PLL operating frequency. The allowed
   * range depends on the device type:
   * - For connectivity-line devices: multiplier range is 4 to 9 (inclusive).
   * - For other devices: multiplier range is 2 to 16 (inclusive).
   *
   * @note The final output frequency is determined by the formula:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input reference clock for the PLL. Available
   * options vary by device type:
   * - For connectivity-line devices:
   *   - @b CLOCK_INTERNAL: uses the on-chip internal oscillator.
   *   - @b CLOCK_EXTERNAL: uses an external clock source, which can be either
   *     an external reference clock signal or an external crystal oscillator.
   *   - @b CLOCK_PLL2: uses the output of PLL2 as the reference clock.
   * - For low-, medium-, high- and XL-density devices:
   *   - @b CLOCK_INTERNAL: uses the on-chip internal oscillator.
   *   - @b CLOCK_EXTERNAL: uses an external clock source.
   */
  enum ClockSource source;
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
enum [[gnu::packed]] UsbClockPrescaler
{
  /** PLL clock is not divided. */
  USB_CLK_DIV_1,
  /** PLL clock is divided by 1.5. */
  USB_CLK_DIV_1_5
};

struct UsbClockConfig
{
  /** Mandatory: PLL clock prescaler. */
  enum UsbClockPrescaler divisor;
};

/* Requires a UsbClockConfig structure */
extern const struct ClockClass * const UsbClock;
/*----------------------------------------------------------------------------*/
struct BusClockConfig
{
  /**
   * Mandatory: bus clock divisor.
   *
   * This field defines the division factor applied to the source clock to
   * generate the target bus clock frequency. The set of valid divisor values
   * depends on the specific bus domain:
   * - @b AHB bus: supported divisors are 2, 4, 8, 16, 64, 128, 256 and 512.
   * - Both @b APB buses: supported divisors are limited to 2, 4, 8 and 16.
   * - @b ADC clock: supported divisors are 2, 4, 6 and 8.
   */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const AdcClock;
extern const struct ClockClass * const Apb1Clock;
extern const struct ClockClass * const Apb2Clock;
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F1XX_CLOCKING_H_ */
