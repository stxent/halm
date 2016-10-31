/*
 * halm/platform/stm/stm32f1xx/clocking.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Clock configuration functions for STM32F1xx series.
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_CLOCKING_H_
#define HALM_PLATFORM_STM_STM32F1XX_CLOCKING_H_
/*----------------------------------------------------------------------------*/
#include <halm/clock.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
//extern const struct ClockClass * const ExternalOsc;
//extern const struct ClockClass * const InternalOsc;
//extern const struct ClockClass * const RtcOsc;
//extern const struct ClockClass * const SystemPll;
//extern const struct ClockClass * const UsbPll;
/*----------------------------------------------------------------------------*/
extern const struct ClockClass * const MainClock;
extern const struct ClockClass * const Apb1Clock;
extern const struct ClockClass * const Apb2Clock;
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_CLOCKING_H_ */
