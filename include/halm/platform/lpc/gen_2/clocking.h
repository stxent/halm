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
  /**
   * Optional: input clock divisor.
   *
   * This field defines the division factor applied to the input clock before
   * generating the output clock signal. The valid divisor range is
   * from 1 to 255 (inclusive). A divisor of 1 results in no division.
   */
  uint16_t divisor;

  /**
   * Mandatory: output pin.
   *
   * Specifies the GPIO pin number used to output the generated clock signal.
   * The exact pin mapping and availability depend on the device configuration.
   */
  PinNumber pin;

  /**
   * Mandatory: clock source selection.
   *
   * Specifies the source of the input reference clock used to generate the
   * output clock signal. The set of available sources may vary depending
   * on the device capabilities.
   */
  enum ClockSource source;
};

struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the crystal oscillator or external clock source.
   *
   * This field specifies the operating frequency of the external clock input,
   * which may be either a crystal oscillator or an external clock signal.
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

struct PllConfig
{
  /**
   * Mandatory: PLL output divisor.
   *
   * This field defines the division factor applied to the PLL output frequency.
   * Only specific even values are supported: 2, 4, 8 and 16.
   */
  uint16_t divisor;

  /**
   * Mandatory: input clock multiplier.
   *
   * This field specifies the multiplication factor applied to the input clock
   * to achieve the desired PLL intermediate frequency. Key constraints:
   * - The resulting frequency after multiplication must be in the range of
   *   156 MHz to 320 MHz.
   * - Valid multiplier values are integers from 1 to 32 (inclusive).
   * - The input clock frequency must be within the range of 10 MHz to 25 MHz.
   *
   * @note The actual output frequency is calculated as:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * - Available options for **System PLL**:
   *   - @b CLOCK_INTERNAL
   *   - @b CLOCK_EXTERNAL
   * - Available options for **USB PLL**:
   *   - @b CLOCK_EXTERNAL
   */
  enum ClockSource source;
};

struct WdtOscConfig
{
  /**
   * Optional: clock frequency divisor. The valid divisor values are constrained
   * to the range from 2 to 64, inclusive, and must be specified in steps of 2.
   */
  uint16_t divisor;

  /** Optional: oscillator frequency selection. */
  enum WdtFrequency frequency;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_CLOCKING_H_ */
