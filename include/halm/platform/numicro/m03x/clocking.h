/*
 * halm/platform/numicro/m03x/clocking.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for Nuvoton M031/M032 chips.
 */

#ifndef HALM_PLATFORM_NUMICRO_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_CLOCKING_H_
#define HALM_PLATFORM_NUMICRO_M03X_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,     /* HIRC */
  CLOCK_INTERNAL_LS,  /* LIRC */
  CLOCK_EXTERNAL,     /* HXT */
  CLOCK_RTC,          /* LXT */
  CLOCK_PLL,
  CLOCK_MAIN,
  CLOCK_APB,
  CLOCK_TMR,

  /* Undefined clock source for internal usage */
  CLOCK_UNDEFINED
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
enum [[gnu::packed]] RtcOscGain
{
  RTC_GAIN_LOW,
  RTC_GAIN_HIGH
};

struct RtcOscConfig
{
  /**
   * Mandatory: gain of the oscillator. Select low gain for crystals with
   * ESR of 35 kOhm and high gain for ESR of 70 kOhm.
   */
  enum RtcOscGain gain;
  /**
   * Mandatory: enable bypass. Bypassing should be enabled when using
   * an external clock source instead of the crystal oscillator.
   */
  bool bypass;
};

/* Requires an RtcOscConfig structure */
extern const struct ClockClass * const RtcOsc;
/*----------------------------------------------------------------------------*/
/* May be initialized with the null pointer */
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const InternalLowSpeedOsc;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: PLL output divisor.
   *
   * This field defines the division factor applied to the PLL output frequency.
   * The supported values are limited to the following options: 1, 2, 4.
   */
  uint16_t divisor;

  /**
   * Mandatory: input clock multiplier.
   *
   * This field specifies the multiplication factor applied to the input clock
   * to achieve the desired PLL intermediate frequency. Key constraints:
   * - The resulting frequency after multiplication should be in the range
   *   of 200 MHz to 500 MHz.
   * - For optimal performance, it is recommended to keep the result within
   *   the narrower range of 250 MHz to 500 MHz.
   * - Valid multiplier values are integers from 2 to 513 (inclusive).
   * - The input clock frequency must be within the range of 3.2 MHz to 150 MHz.
   *
   * @note The final output frequency is determined by the formula:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Available options for **System PLL**:
   * - @b CLOCK_INTERNAL divided by 4
   * - @b CLOCK_EXTERNAL
   */
  enum ClockSource source;
};

/* Requires a PllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /**
   * Mandatory: input clock divisor.
   *
   * This field defines the division factor applied to the input clock before
   * generating the output clock signal. Possible values are powers of two
   * from 2 to 65536 (inclusive).
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

/* Requires a ClockOutputConfig structure */
extern const struct ClockClass * const ClockOutput;
/*----------------------------------------------------------------------------*/
struct ApbClockConfig
{
  /**
   * Mandatory: AHB clock divisor. The divisor may be set
   * to divide by 1, 2, 4, 8, 16.
   */
  uint16_t divisor;
};

struct GenericClockConfig
{
  /** Mandatory: clock source selection. */
  enum ClockSource source;
};

struct ExtendedClockConfig
{
  /** Mandatory: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
  /** Mandatory: clock source selection. */
  enum ClockSource source;
};

/* Requires an ApbClockConfig structure */
extern const struct ClockClass * const Apb0Clock;
extern const struct ClockClass * const Apb1Clock;

/* Requires a GenericClockConfig structure */
extern const struct ClockClass * const Bpwm0Clock;
extern const struct ClockClass * const Bpwm1Clock;
extern const struct ClockClass * const Pwm0Clock;
extern const struct ClockClass * const Pwm1Clock;
extern const struct ClockClass * const Qspi0Clock;
extern const struct ClockClass * const Spi0Clock;
extern const struct ClockClass * const SysTickClock;
extern const struct ClockClass * const Timer0Clock;
extern const struct ClockClass * const Timer1Clock;
extern const struct ClockClass * const Timer2Clock;
extern const struct ClockClass * const Timer3Clock;
extern const struct ClockClass * const WdtClock;
extern const struct ClockClass * const WwdtClock;

/* Requires an ExtendedClockConfig structure */
extern const struct ClockClass * const AdcClock;
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const Uart0Clock;
extern const struct ClockClass * const Uart1Clock;
extern const struct ClockClass * const Uart2Clock;
extern const struct ClockClass * const Uart3Clock;
extern const struct ClockClass * const Uart4Clock;
extern const struct ClockClass * const Uart5Clock;
extern const struct ClockClass * const Uart6Clock;
extern const struct ClockClass * const Uart7Clock;
extern const struct ClockClass * const UsbClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_CLOCKING_H_ */
