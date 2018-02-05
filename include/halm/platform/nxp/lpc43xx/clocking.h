/*
 * halm/platform/nxp/lpc43xx/clocking.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for LPC43xx series.
 */

#ifndef HALM_PLATFORM_NXP_LPC43XX_CLOCKING_H_
#define HALM_PLATFORM_NXP_LPC43XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum ClockBranch
{
  CLOCK_BASE_USB0     = 0,
  CLOCK_BASE_PERIPH   = 1,
  CLOCK_BASE_USB1     = 2,
  CLOCK_BASE_M4       = 3,
  CLOCK_BASE_SPIFI    = 4,
  CLOCK_BASE_SPI      = 5,
  CLOCK_BASE_PHY_RX   = 6,
  CLOCK_BASE_PHY_TX   = 7,
  CLOCK_BASE_APB1     = 8,
  CLOCK_BASE_APB3     = 9,
  CLOCK_BASE_LCD      = 10,
  CLOCK_BASE_ADCHS    = 11,
  CLOCK_BASE_SDIO     = 12,
  CLOCK_BASE_SSP0     = 13,
  CLOCK_BASE_SSP1     = 14,
  CLOCK_BASE_USART0   = 15,
  CLOCK_BASE_UART1    = 16,
  CLOCK_BASE_USART2   = 17,
  CLOCK_BASE_USART3   = 18,
  CLOCK_BASE_OUT      = 19,
  CLOCK_BASE_AUDIO    = 24,
  CLOCK_BASE_CGU_OUT0 = 25,
  CLOCK_BASE_CGU_OUT1 = 26
};

enum ClockSource
{
  CLOCK_RTC         = 0x00,
  CLOCK_INTERNAL    = 0x01,
  CLOCK_ENET_RX     = 0x02,
  CLOCK_ENET_TX     = 0x03,
  CLOCK_GP_CLKIN    = 0x04,
  CLOCK_EXTERNAL    = 0x06,
  CLOCK_USB_PLL     = 0x07,
  CLOCK_AUDIO_PLL   = 0x08,
  CLOCK_PLL         = 0x09,
  CLOCK_IDIVA       = 0x0C,
  CLOCK_IDIVB       = 0x0D,
  CLOCK_IDIVC       = 0x0E,
  CLOCK_IDIVD       = 0x0F,
  CLOCK_IDIVE       = 0x10
};

struct GenericClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
};

struct GenericDividerClass
{
  struct ClockClass base;
  enum ClockSource channel;
};

struct GenericPllClass
{
  struct ClockClass base;
  enum ClockSource channel;
};
/*----------------------------------------------------------------------------*/
struct GenericDividerConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /**
   * Mandatory: integer divider value.
   * @n The divider value for the Divider A should be in the range of 1 to 4.
   * @n Divider values for Dividers B, C, D should be in the range of 1 to 16.
   * @n The divider value for the Divider E should be in the range of 1 to 256.
   */
  uint16_t divisor;
};

/* Require a GenericDividerConfig structure */
extern const struct GenericDividerClass * const DividerA;
extern const struct GenericDividerClass * const DividerB;
extern const struct GenericDividerClass * const DividerC;
extern const struct GenericDividerClass * const DividerD;
extern const struct GenericDividerClass * const DividerE;
/*----------------------------------------------------------------------------*/
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

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;
/*----------------------------------------------------------------------------*/
/* May be initialized with the null pointer */
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const RtcOsc;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: clock source.
   * @n The input frequency range for Audio PLL (PLL0AUDIO)
   * and USB PLL (PLL0USB) is 14 kHz to 150 MHz. The input from
   * an external crystal is limited to 25 MHz.
   * @n The range for System PLL (PLL1) is 1 to 50 MHz. The input from
   * an external crystal is limited to 25 MHz.
   **/
  enum ClockSource source;
  /**
   * Mandatory: input clock multiplier.
   * @n Audio PLL and USB PLL operate in the range of 275 MHz to 550 MHz,
   * multiplier range is 1 to 32768. To get the best phase-noise and
   * jitter performance, the even value has to be used.
   * @n System PLL operates in the range of 156 MHz to 320 MHz,
   * multiplier range is 1 to 256.
   */
  uint16_t multiplier;
  /**
   * Mandatory: PLL output divisor.
   * @n Audio PLL and USB PLL divisor must be set to 1 or be in the range
   * of 2 to 64 in steps of 2.
   * @n System PLL accepts a limited set of values: 1, 2, 4, 8 and 16.
   */
  uint16_t divisor;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const SystemPll;
extern const struct GenericPllClass * const AudioPll;
extern const struct GenericPllClass * const UsbPll;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Require a GenericClockConfig structure */
extern const struct GenericClockClass * const MainClock; /* Base M4 clock */
extern const struct GenericClockClass * const Usb0Clock;
extern const struct GenericClockClass * const Usb1Clock;
extern const struct GenericClockClass * const PeriphClock; /* APB0 and APB2 */
extern const struct GenericClockClass * const Apb1Clock;
extern const struct GenericClockClass * const Apb3Clock;
extern const struct GenericClockClass * const SpifiClock;
extern const struct GenericClockClass * const SpiClock;
extern const struct GenericClockClass * const PhyRxClock;
extern const struct GenericClockClass * const PhyTxClock;
extern const struct GenericClockClass * const LcdClock;
extern const struct GenericClockClass * const AdcHsClock;
extern const struct GenericClockClass * const SdioClock;
extern const struct GenericClockClass * const Ssp0Clock;
extern const struct GenericClockClass * const Ssp1Clock;
extern const struct GenericClockClass * const Usart0Clock;
extern const struct GenericClockClass * const Uart1Clock;
extern const struct GenericClockClass * const Usart2Clock;
extern const struct GenericClockClass * const Usart3Clock;
extern const struct GenericClockClass * const AudioClock;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /** Mandatory: output pin. */
  PinNumber pin;
};

/* Require a ClockOutputConfig structure */
extern const struct GenericClockClass * const ClockOutput;
extern const struct GenericClockClass * const CguOut0Clock;
extern const struct GenericClockClass * const CguOut1Clock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_CLOCKING_H_ */
