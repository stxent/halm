/*
 * halm/platform/stm32/stm32f0xx/pin.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_PIN_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_PIN_H_
#define HALM_PLATFORM_STM32_STM32F0XX_PIN_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  PORT_A,
  PORT_B,
  PORT_C,
  PORT_D,
  PORT_E,
  PORT_F
};
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_PIN/pin.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_PIN_H_ */
