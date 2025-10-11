/*
 * halm/platform/lpc/gen_2/clocking.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions.
 */

#ifndef HALM_PLATFORM_LPC_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_CLOCKING_H_
#define HALM_PLATFORM_LPC_GEN_2_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] WdtFrequency
{
  /* Watchdog oscillator analog output frequency in kHz */
  WDT_FREQ_DEFAULT,
  WDT_FREQ_600,
  WDT_FREQ_1050,
  WDT_FREQ_1400,
  WDT_FREQ_1750,
  WDT_FREQ_2100,
  WDT_FREQ_2400,
  WDT_FREQ_2700,
  WDT_FREQ_3000,
  WDT_FREQ_3250,
  WDT_FREQ_3500,
  WDT_FREQ_3750,
  WDT_FREQ_4000,
  WDT_FREQ_4200,
  WDT_FREQ_4400,
  WDT_FREQ_4600
};
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Optional: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: output pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source.
   */
  uint32_t frequency;
  /**
   * Optional: enable bypass. Bypassing should be enabled when using
   * an external clock source instead of the crystal oscillator.
   */
  bool bypass;
};

struct GenericClockConfig
{
  /** Optional: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

struct PllConfig
{
  /**
   * Mandatory: PLL output divisor. The output divisor may be set
   * to divide by 2, 4, 8, 16.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier, result should be in the range from
   * 156 MHz to 320 MHz. Multiplier range is 1 to 32. Note that the input
   * frequency range is 10 to 25 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   * @n Available options for USB PLL:
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
};

struct WdtOscConfig
{
  /**
   * Optional: clock frequency divisor. Divisor range is 2 to 64 in step of 2.
   */
  uint16_t divisor;
  /** Optional: oscillator frequency. */
  enum WdtFrequency frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_CLOCKING_H_ */
