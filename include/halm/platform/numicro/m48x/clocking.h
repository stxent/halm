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
   * Mandatory: frequency of the external crystal oscillator or
   * an external clock source.
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
   * Mandatory: PLL output divisor. The output divisor may be set
   * to divide by 1, 2, 4.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier, result should be in the range from
   * 200 MHz to 500 MHz. Multiplier range is 2 to 513. Note that the input
   * frequency range is 4 to 24 MHz.
   */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n Available options for System PLL:
   *   - @b CLOCK_INTERNAL.
   *   - @b CLOCK_EXTERNAL.
   */
  enum ClockSource source;
};

/* Requires a PllConfig structure */
extern const struct ClockClass * const SystemPll;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /**
   * Mandatory: input clock divisor. Possible values are
   * powers of two from 2 to 65536.
   */
  uint16_t divisor;
  /** Mandatory: output pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
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
  /** Mandatory: clock source. */
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
  /** Mandatory: clock source. */
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

/* SDH dividers are used by SDH driver */
extern const struct ClockClass * const Sdh0Clock;
extern const struct ClockClass * const Sdh1Clock;

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
