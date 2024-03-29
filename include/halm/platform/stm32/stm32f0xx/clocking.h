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
   * Mandatory: clock source.
   * @n Available options:
   *   - @b ADC_CLOCK_INTERNAL_14.
   *   - @b ADC_CLOCK_APB_DIV_2.
   *   - @b ADC_CLOCK_APB_DIV_4.
   */
  enum AdcClockSource source;
};

/* Requires an AdcClockConfig structure */
extern const struct ClockClass * const AdcClock;
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
extern const struct ClockClass * const InternalLowSpeedOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const InternalOsc14;
extern const struct ClockClass * const InternalOsc48;
/*----------------------------------------------------------------------------*/
struct SystemPllConfig
{
  /** Mandatory: PLL input divisor. Divisor range is 1 to 16. */
  uint16_t divisor;
  /** Mandatory: PLL multiplier. The multiplier range is 2 to 16. */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n Available options:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_INTERNAL_48.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
};

/* Requires a SystemPllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /**
   * Mandatory: clock source.
   * @n Available sources for system clock are:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_INTERNAL_48.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_PLL.
   * @n Available sources for I2C1 interface are:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_SYSTEM.
   * @n Available sources for USART1, USART2 and USART3 interfaces are:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_RTC.
   *   - @b CLOCK_APB.
   *   - @b CLOCK_SYSTEM.
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
   * @n Available options for AHB: 2, 4, 8, 16, 64, 128, 256, 512.
   * @n Available options for APB: 2, 4, 8, 16.
   */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const ApbClock;
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_CLOCKING_H_ */
