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
/*----------------------------------------------------------------------------*/
struct CommonClockClass
{
  struct ClockClass base;

  enum ClockBranch branch;
};
/*----------------------------------------------------------------------------*/
struct CommonDividerClass
{
  struct ClockClass base;

  enum ClockSource channel;
};
/*----------------------------------------------------------------------------*/
extern const struct CommonDividerClass * const DividerA;
extern const struct CommonDividerClass * const DividerB;
extern const struct CommonDividerClass * const DividerC;
extern const struct CommonDividerClass * const DividerD;
extern const struct CommonDividerClass * const DividerE;
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const ExternalOsc;
extern const struct ClockClass * const InternalOsc;
extern const struct ClockClass * const RtcOsc;
extern const struct ClockClass * const AudioPll;
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
/*----------------------------------------------------------------------------*/
/* Base M4 clock */
extern const struct CommonClockClass * const MainClock;
extern const struct CommonClockClass * const Usb0Clock;
extern const struct CommonClockClass * const Usb1Clock;
/* APB0 and APB2 clocks */
extern const struct CommonClockClass * const PeriphClock;
extern const struct CommonClockClass * const Apb1Clock;
extern const struct CommonClockClass * const Apb3Clock;
extern const struct CommonClockClass * const SpifiClock;
extern const struct CommonClockClass * const SpiClock;
extern const struct CommonClockClass * const PhyRxClock;
extern const struct CommonClockClass * const PhyTxClock;
extern const struct CommonClockClass * const LcdClock;
extern const struct CommonClockClass * const AdcHsClock;
extern const struct CommonClockClass * const SdioClock;
extern const struct CommonClockClass * const Ssp0Clock;
extern const struct CommonClockClass * const Ssp1Clock;
extern const struct CommonClockClass * const Usart0Clock;
extern const struct CommonClockClass * const Uart1Clock;
extern const struct CommonClockClass * const Usart2Clock;
extern const struct CommonClockClass * const Usart3Clock;
extern const struct CommonClockClass * const AudioClock;
extern const struct CommonClockClass * const ClockOutput;
extern const struct CommonClockClass * const CguOut0Clock;
extern const struct CommonClockClass * const CguOut1Clock;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /** Mandatory: output pin. */
  PinNumber pin;
};
/*----------------------------------------------------------------------------*/
struct CommonClockConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
};
/*----------------------------------------------------------------------------*/
struct CommonDividerConfig
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
/*----------------------------------------------------------------------------*/
struct ExternalOscConfig
{
  /**
   * Mandatory: frequency of the external crystal oscillator or
   * external clock source.
   */
  uint32_t frequency;
  /**
   * Optional: enable bypass. Bypassing should be enabled when using
   * an external clock source instead of the crystal oscillator.
   */
  bool bypass;
};
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
  /**
   * Mandatory: PLL output divisor. The output divisor may be set to divide
   * by 1, 2, 3, 4, 6, 8, 12, 16, 24, 32.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier, result should be in the range of
   * 156 MHz to 320 MHz. Multiplier range is 1 to 256. Note that the input
   * frequency range is 1 to 50 MHz.
   */
  uint16_t multiplier;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC43XX_CLOCKING_H_ */
