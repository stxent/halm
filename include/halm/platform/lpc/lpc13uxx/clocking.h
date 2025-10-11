/*
 * halm/platform/lpc/lpc13uxx/clocking.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

/**
 * @file
 * Clock configuration functions for LPC13Uxx series.
 */

#ifndef HALM_PLATFORM_LPC_CLOCKING_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13UXX_CLOCKING_H_
#define HALM_PLATFORM_LPC_LPC13UXX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] ClockBranch
{
  CLOCK_BRANCH_MAIN,
  CLOCK_BRANCH_OUTPUT,
  CLOCK_BRANCH_USB
};

enum [[gnu::packed]] ClockSource
{
  CLOCK_INTERNAL,
  CLOCK_EXTERNAL,
  CLOCK_PLL,
  CLOCK_USB_PLL,
  CLOCK_WDT,
  CLOCK_MAIN
};
/*----------------------------------------------------------------------------*/
/* Requires a ClockOutputConfig structure */
extern const struct ClockClass * const ClockOutput;

/* Requires an ExternalOscConfig structure */
extern const struct ClockClass * const ExternalOsc;

/* May be initialized with the null pointer */
extern const struct ClockClass * const InternalOsc;

/* Require a GenericClockConfig structure */
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const UsbClock;

/* Mock clock source, configuration is not needed */
extern const struct ClockClass * const SystemClock;

/* Require a PllConfig structure */
extern const struct ClockClass * const SystemPll;
extern const struct ClockClass * const UsbPll;

/* Requires a WdtOscConfig structure */
extern const struct ClockClass * const WdtOsc;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_CLOCK/clocking.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_CLOCKING_H_ */
