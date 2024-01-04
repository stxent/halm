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
/*----------------------------------------------------------------------------*/
enum ClockSource
{
  CLOCK_INTERNAL,    /* HSI */
  CLOCK_INTERNAL_LS, /* LSI */
  CLOCK_EXTERNAL,    /* HSE */
  CLOCK_I2S_PLL,
  CLOCK_PLL,
  CLOCK_RTC,         /* LSE */
  CLOCK_SYSTEM
} __attribute__((packed));

enum VoltageRange
{
  VR_DEFAULT,
  VR_1V8_2V1,
  VR_2V1_2V4,
  VR_2V4_2V7,
  VR_2V7_3V6
} __attribute__((packed));
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
struct MainPllConfig
{
  /** Mandatory: PLL main output divisor. Possible values are 2, 4, 6 and 8. */
  uint16_t divisor;
  /**
   * Mandatory: PLL multiplier. PLL VCO frequency should be in the range
   * of 64 to 432 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n Available options:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
};

/* Requires a MainPllConfig structure */
extern const struct ClockClass * const MainPll;
/*----------------------------------------------------------------------------*/
struct SystemClockConfig
{
  /**
   * Mandatory: system clock source.
   * @n Available sources are:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   *   - @b CLOCK_PLL.
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
