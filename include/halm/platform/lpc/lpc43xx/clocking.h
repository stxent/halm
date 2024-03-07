/*
 * halm/platform/lpc/lpc43xx/clocking.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for LPC43xx series.
 */

#ifndef HALM_PLATFORM_LPC_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_CLOCKING_H_
#define HALM_PLATFORM_LPC_LPC43XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockBranch
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

enum [[gnu::packed]] ClockSource
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
struct GenericDividerConfig
{
  /**
   * Mandatory: integer divider value.
   * @n The divider value for the Divider A should be in the range from 1 to 4.
   * @n Divider values for Dividers B, C, D should be in the range from 1 to 16.
   * @n The divider value for the Divider E should be in the range from 1 to 256.
   */
  uint16_t divisor;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Require a GenericDividerConfig structure */
extern const struct ClockClass * const DividerA;
extern const struct ClockClass * const DividerB;
extern const struct ClockClass * const DividerC;
extern const struct ClockClass * const DividerD;
extern const struct ClockClass * const DividerE;
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
   * Mandatory: PLL output divisor.
   * @n Audio PLL and USB PLL divisor must be set to 1 or be in the range
   * of 2 to 64 in steps of 2.
   * @n System PLL accepts a limited set of values: 1, 2, 4, 8 and 16.
   */
  uint16_t divisor;
  /**
   * Mandatory: input clock multiplier.
   * @n Audio PLL and USB PLL operate in the range from 275 MHz to 550 MHz,
   * multiplier range is 1 to 32768. To get the best phase-noise and
   * jitter performance, the even value has to be used.
   * @n System PLL operates in the range from 156 MHz to 320 MHz,
   * multiplier range is 1 to 256.
   */
  uint16_t multiplier;
  /**
   * Mandatory: clock source.
   * @n The input frequency range for Audio PLL (PLL0AUDIO)
   * and USB PLL (PLL0USB) is 14 kHz to 150 MHz. The input from
   * an external crystal is limited to 25 MHz.
   * @n The range for System PLL (PLL1) is 1 to 50 MHz. The input from
   * an external crystal is limited to 25 MHz.
   **/
  enum ClockSource source;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const AudioPll;
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Require a GenericClockConfig structure */

/* Base M4 clock */
extern const struct ClockClass * const AdcHsClock;
extern const struct ClockClass * const Apb1Clock;
extern const struct ClockClass * const Apb3Clock;
extern const struct ClockClass * const AudioClock;
/* APB0, APB2, Cortex-M0 and SGPIO */
extern const struct ClockClass * const LcdClock;
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const PeriphClock;
extern const struct ClockClass * const PhyRxClock;
extern const struct ClockClass * const PhyTxClock;
extern const struct ClockClass * const SdioClock;
extern const struct ClockClass * const SpiClock;
extern const struct ClockClass * const SpifiClock;
extern const struct ClockClass * const Ssp0Clock;
extern const struct ClockClass * const Ssp1Clock;
extern const struct ClockClass * const Uart1Clock;
extern const struct ClockClass * const Usart0Clock;
extern const struct ClockClass * const Usart2Clock;
extern const struct ClockClass * const Usart3Clock;
extern const struct ClockClass * const Usb0Clock;
extern const struct ClockClass * const Usb1Clock;
/*----------------------------------------------------------------------------*/
struct ClockOutputConfig
{
  /** Mandatory: output pin. */
  PinNumber pin;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* Require a ClockOutputConfig structure */
extern const struct ClockClass * const CguOut0Clock;
extern const struct ClockClass * const CguOut1Clock;
extern const struct ClockClass * const ClockOutput;
/*----------------------------------------------------------------------------*/
struct [[gnu::packed]] ClockSettings
{
  uint32_t extOscFrequency;
  uint32_t audioPllFrequency;
  uint32_t sysPllFrequency;
  uint32_t usbPllFrequency;
  uint32_t mainClockFrequency;
  uint32_t checksum;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

bool loadClockSettings(const struct ClockSettings *);
void storeClockSettings(struct ClockSettings *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_CLOCKING_H_ */
