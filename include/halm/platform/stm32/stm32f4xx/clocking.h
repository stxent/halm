/*
 * halm/platform/stm32/stm32f4xx/clocking.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for STM32F4xx series.
 */

#ifndef HALM_PLATFORM_STM32_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_H_
#define HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
#include <halm/platform/stm32/system.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,    /* HSI */
  CLOCK_INTERNAL_LS, /* LSI */
  CLOCK_EXTERNAL,    /* HSE */
  CLOCK_I2S_PLL,
  CLOCK_PLL,
  CLOCK_RTC,         /* LSE */
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
/*----------------------------------------------------------------------------*/
struct MainClockConfig
{
  /**
   * Mandatory: clock divisor for AHB. Possible values are
   * 2, 4, 8, 16, 64, 128, 256 and 512.
   */
  uint16_t divisor;
  /** Optional: voltage range. */
  enum VoltageRange range;
};

/* Require a MainClockConfig structure */
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: PLL output divisor.
   *
   * This field defines the division factor applied to the PLL output frequency.
   * The valid range depends on the PLL type:
   * - **Audio PLL**: the divisor must be in the range of 2 to 7.
   * - **System PLL**: only specific even values are supported: 2, 4, 6, 8.
   */
  uint16_t divisor;

  /**
   * Mandatory: input clock multiplier. This field specifies the multiplication
   * factor applied to the input clock to achieve the desired PLL operating
   * frequency. The effective frequency range
   * - **Audio PLL**: operates within the range of 192 MHz to 432 MHz.
   * - **System PLL**: operates within the range of 64 MHz to 432 MHz.
   *
   * @note The actual output frequency is calculated as:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input clock signal for the PLL.
   * Available options:
   * - @b CLOCK_INTERNAL: uses the on-chip internal oscillator.
   * - @b CLOCK_EXTERNAL: uses an external clock signal.
   */
  enum ClockSource source;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const AudioPll;
extern const struct ClockClass * const MainPll;
/*----------------------------------------------------------------------------*/
struct SystemClockConfig
{
  /**
   * Mandatory: system clock source.
   *
   * Available sources are:
   * - @b CLOCK_INTERNAL
   * - @b CLOCK_EXTERNAL
   * - @b CLOCK_PLL
   */
  enum ClockSource source;
};

/* Requires a SystemClockConfig structure */
extern const struct ClockClass * const SystemClock;
/*----------------------------------------------------------------------------*/
/* Stub for PLL48CLK frequency, does not require initialization */
extern const struct ClockClass * const UsbClock;
/*----------------------------------------------------------------------------*/
struct BusClockConfig
{
  /** Mandatory: clock divisor for APB bus. Possible values are 2, 4, 8, 16. */
  uint16_t divisor;
};

/* Require a BusClockConfig structure */
extern const struct ClockClass * const Apb1Clock;
extern const struct ClockClass * const Apb2Clock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_CLOCKING_H_ */
