/*
 * halm/platform/stm32/iwdg.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_IWDG_H_
#define HALM_PLATFORM_STM32_IWDG_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/iwdg_base.h>
#include <halm/watchdog.h>
/*----------------------------------------------------------------------------*/
extern const struct WatchdogClass * const Iwdg;

struct IwdgConfig
{
  /** Mandatory: timer period in milliseconds. */
  uint32_t period;
};

struct Iwdg
{
  struct IwdgBase base;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_IWDG_H_ */
