/*
 * halm/platform/numicro/m48x/clocking.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for Nuvoton M48x chips.
 */

#ifndef HALM_PLATFORM_NUMICRO_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_CLOCKING_H_
#define HALM_PLATFORM_NUMICRO_M48X_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,     /* HIRC */
  CLOCK_INTERNAL_HS,  /* HIRC 48M */
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
   * The supported frequency range is strictly limited to 4 MHz to 24 MHz
   * (inclusive).
   */
  uint32_t frequency;
};

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;
/*----------------------------------------------------------------------------*/
/* May be initialized with the null pointer */
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const InternalHighSpeedOsc;
extern const struct ClockClass * const InternalLowSpeedOsc;
extern const struct ClockClass * const RtcOsc;
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
   * - Valid multiplier values are integers from 2 to 513 (inclusive).
   * - The input clock frequency must be within the range of 4 MHz to 24 MHz.
   *
   * @note The final output frequency is determined by the formula:
   *       `output = (input * multiplier) / divisor`.
   */
  uint16_t multiplier;

  /**
   * Mandatory: clock source selection.
   *
   * Available options for **System PLL**:
   * - @b CLOCK_INTERNAL
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

struct DividedClockConfig
{
  /** Mandatory: input clock divisor in the range from 1 to 255. */
  uint16_t divisor;
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
extern const struct ClockClass * const Epwm0Clock;
extern const struct ClockClass * const Epwm1Clock;
extern const struct ClockClass * const Qspi0Clock;
extern const struct ClockClass * const Qspi1Clock;
extern const struct ClockClass * const RtcClock;
extern const struct ClockClass * const Spi0Clock;
extern const struct ClockClass * const Spi1Clock;
extern const struct ClockClass * const Spi2Clock;
extern const struct ClockClass * const Spi3Clock;
extern const struct ClockClass * const SysTickClock;
extern const struct ClockClass * const Timer0Clock;
extern const struct ClockClass * const Timer1Clock;
extern const struct ClockClass * const Timer2Clock;
extern const struct ClockClass * const Timer3Clock;
extern const struct ClockClass * const WdtClock;
extern const struct ClockClass * const WwdtClock;

/* Require a GenericClockConfig structure, SDH clocks are used by SDH driver */
extern const struct ClockClass * const Sdh0Clock;
extern const struct ClockClass * const Sdh1Clock;

/* Requires an ExtendedClockConfig structure */
extern const struct ClockClass * const CcapClock;
extern const struct ClockClass * const I2S0Clock;
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const Sc0Clock;
extern const struct ClockClass * const Sc1Clock;
extern const struct ClockClass * const Sc2Clock;
extern const struct ClockClass * const Uart0Clock;
extern const struct ClockClass * const Uart1Clock;
extern const struct ClockClass * const Uart2Clock;
extern const struct ClockClass * const Uart3Clock;
extern const struct ClockClass * const Uart4Clock;
extern const struct ClockClass * const Uart5Clock;
extern const struct ClockClass * const Uart6Clock;
extern const struct ClockClass * const Uart7Clock;
extern const struct ClockClass * const UsbClock;
extern const struct ClockClass * const VsenseClock;

/* Requires a DividedClockConfig structure */
extern const struct ClockClass * const Eadc0Clock;
extern const struct ClockClass * const Eadc1Clock;
extern const struct ClockClass * const EmacClock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_CLOCKING_H_ */
