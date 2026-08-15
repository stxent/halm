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
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,    /* HSI */
  CLOCK_INTERNAL_14, /* HSI14 */
  CLOCK_INTERNAL_48, /* HSI48 */
  CLOCK_INTERNAL_LS, /* LSI */
  CLOCK_EXTERNAL,    /* HSE */
  CLOCK_PLL,
  CLOCK_RTC,         /* LSE */
  CLOCK_APB,
  CLOCK_SYSTEM
};
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] AdcClockSource
{
  /** Separate asynchronous ADC clock. */
  ADC_CLOCK_INTERNAL_14,
  /** Synchronous APB clock divided by 2. */
  ADC_CLOCK_APB_DIV_2,
  /** Synchronous APB clock divided by 4. */
  ADC_CLOCK_APB_DIV_4
};

struct AdcClockConfig
{
  /**
   * Mandatory: clock source selection.
   *
   * Available options:
   * - @b ADC_CLOCK_INTERNAL_14
   * - @b ADC_CLOCK_APB_DIV_2
   * - @b ADC_CLOCK_APB_DIV_4
   */
  enum AdcClockSource source;
};

/* Requires an AdcClockConfig structure */
extern const struct ClockClass * const AdcClock;
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the crystal oscillator or external clock source.
   *
   * This field specifies the operating frequency of the external clock input,
   * which may be either a crystal oscillator or an external clock signal.
   * The supported frequency range is strictly limited to 4 MHz to 32 MHz
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
extern const struct ClockClass * const InternalLowSpeedOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const InternalOsc14;
extern const struct ClockClass * const InternalOsc48;
/*----------------------------------------------------------------------------*/
struct SystemPllConfig
{
  /**
   * Mandatory: PLL input divisor.
   *
   * This field defines the division factor applied to the input clock before
   * it is fed into the PLL. The valid divisor may be set to any integer value
   * in the range of 1 to 16 (inclusive).
   */
  uint16_t divisor;

  /**
   * Mandatory: PLL multiplier.
   *
   * This field specifies the multiplication factor applied to the pre-divided
   * input clock to achieve the desired PLL operating frequency. The allowed
   * multiplier range is 2 to 16 (inclusive).
   *
   * @note The final output frequency is determined by the formula:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input reference clock for the PLL. Available
   * options are:
   * - @b CLOCK_INTERNAL
   * - @b CLOCK_INTERNAL_48
   * - @b CLOCK_EXTERNAL
   */
  enum ClockSource source;
};

/* Requires a SystemPllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /**
   * Mandatory: clock source selection.
   *
   * - Available sources for system clock are:
   *   - @b CLOCK_INTERNAL
   *   - @b CLOCK_INTERNAL_48
   *   - @b CLOCK_EXTERNAL
   *   - @b CLOCK_PLL
   * - Available sources for I2C1 interface are:
   *   - @b CLOCK_INTERNAL
   *   - @b CLOCK_SYSTEM
   * - Available sources for USART1, USART2 and USART3 interfaces are:
   *   - @b CLOCK_INTERNAL
   *   - @b CLOCK_RTC
   *   - @b CLOCK_APB
   *   - @b CLOCK_SYSTEM
   */
  enum ClockSource source;
};

/* Requires a GenericClockConfig structure */
extern const struct ClockClass * const I2C1Clock;
extern const struct ClockClass * const SystemClock;
extern const struct ClockClass * const Usart1Clock;
extern const struct ClockClass * const Usart2Clock;
extern const struct ClockClass * const Usart3Clock;
/*----------------------------------------------------------------------------*/
struct BusClockConfig
{
  /**
   * Mandatory: bus clock divisor.
   *
   * - Available options for @b AHB: 2, 4, 8, 16, 64, 128, 256, 512.
   * - Available options for @b APB: 2, 4, 8, 16.
   */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const ApbClock;
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_H_ */
