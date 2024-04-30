/*
 * halm/platform/imxrt/imxrt106x/clocking.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for i.MX RT106x series.
 */

#ifndef HALM_PLATFORM_IMXRT_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockSource
{
  CLOCK_OSC,
  CLOCK_RTC,

  CLOCK_ARM_PLL, /* PLL1 */
  CLOCK_AUDIO_PLL, /* PLL4 */
  CLOCK_ETHERNET_PLL, /* PLL6 */
  CLOCK_SYSTEM_PLL, /* PLL2 */
  CLOCK_SYSTEM_PLL_PFD0, /* PLL2 PFD0 */
  CLOCK_SYSTEM_PLL_PFD1, /* PLL2 PFD1 */
  CLOCK_SYSTEM_PLL_PFD2, /* PLL2 PFD2 */
  CLOCK_SYSTEM_PLL_PFD3, /* PLL2 PFD3 - unused */
  CLOCK_USB1_PLL, /* PLL3 */
  CLOCK_USB1_PLL_PFD0, /* PLL3 PFD0 */
  CLOCK_USB1_PLL_PFD1, /* PLL3 PFD1 */
  CLOCK_USB1_PLL_PFD2, /* PLL3 PFD2 */
  CLOCK_USB1_PLL_PFD3, /* PLL3 PFD3 */
  CLOCK_USB2_PLL, /* PLL7 */
  CLOCK_VIDEO_PLL, /* PLL5 */

  CLOCK_IPG,
  CLOCK_SEMC
};
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ExternalOscCurrent
{
  OSC_CURRENT_NOMINAL,
  OSC_CURRENT_MINUS_12P5,
  OSC_CURRENT_MINUS_25P,
  OSC_CURRENT_MINUS_37P5
};

enum [[gnu::packed]] ExternalOscDelay
{
  OSC_DELAY_DEFAULT,
  OSC_DELAY_0MS25,
  OSC_DELAY_0MS5,
  OSC_DELAY_1MS,
  OSC_DELAY_2MS
};

struct ExternalOscConfig
{
  /** Optional: bias current of the 24MHz oscillator. */
  enum ExternalOscCurrent current;
  /** Optional: startup delay of the 24MHz oscillator. */
  enum ExternalOscDelay delay;
};

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;

/* May be initialized with the null pointer */
extern const struct ClockClass * const InternalOsc;
/*----------------------------------------------------------------------------*/
struct GenericClockConfig
{
  /** Optional: clock divider value. */
  uint16_t divisor;
};

/* IPG_CLK_ROOT - IPG bus clock */
extern const struct ClockClass * const IpgClock;
/* AHB_CLK_ROOT - ARM Core clock */
extern const struct ClockClass * const MainClock;
/*----------------------------------------------------------------------------*/
struct ExtendedClockConfig
{
  /** Optional: clock divider value. */
  uint16_t divisor;
  /** Mandatory: clock source. */
  enum ClockSource source;
};

/* FLEXSPI_CLK_ROOT */
extern const struct ClockClass * const FlexSpi1Clock;
/* FLEXSPI2_CLK_ROOT */
extern const struct ClockClass * const FlexSpi2Clock;
/* PERIPH_CLK */
extern const struct ClockClass * const PeriphClock;
/* PERCLK_CLK_ROOT - PIT, GPT */
extern const struct ClockClass * const TimerClock;
/* UART_CLK_ROOT */
extern const struct ClockClass * const UartClock;
/*----------------------------------------------------------------------------*/
struct PllConfig
{
  /**
   * Mandatory: PLL loop divider value.
   * @n Divider values for PFD should be in the range from 12 to 35.
   * @n The divider value for the PLL1 (ARM PLL) should be in the range
   * from 54 to 108. Output frequency is additionally divided by 2.
   * @n The divider value for the PLL2 (System PLL) should be 20 or 22.
   * @n The divider value for the PLL3 (USB1 PLL) should be 20 or 22.
   * @n The divider value for the PLL7 (USB2 PLL) should be 20 or 22.
   */
  uint16_t divisor;
};

/* Require a PllConfig structure */
extern const struct ClockClass * const ArmPll; /* PLL1 */
extern const struct ClockClass * const AudioPll; /* PLL4 */
extern const struct ClockClass * const EthernetPll; /* PLL6 */
extern const struct ClockClass * const SystemPll; /* PLL2 */
extern const struct ClockClass * const SystemPllPfd0; /* PLL2 PFD0 */
extern const struct ClockClass * const SystemPllPfd1; /* PLL2 PFD1 */
extern const struct ClockClass * const SystemPllPfd2; /* PLL2 PFD2 */
extern const struct ClockClass * const SystemPllPfd3; /* PLL2 PFD3 - unused */
extern const struct ClockClass * const Usb1Pll; /* PLL3 */
extern const struct ClockClass * const Usb1PllPfd0; /* PLL3 PFD0 */
extern const struct ClockClass * const Usb1PllPfd1; /* PLL3 PFD1 */
extern const struct ClockClass * const Usb1PllPfd2; /* PLL3 PFD2 */
extern const struct ClockClass * const Usb1PllPfd3; /* PLL3 PFD3 */
extern const struct ClockClass * const Usb2Pll; /* PLL7 */
extern const struct ClockClass * const VideoPll; /* PLL5 */
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_CLOCKING_H_ */
