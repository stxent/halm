/*
 * halm/platform/stm32/stm32f0xx/pin_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_STM32F0XX_PIN_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F0XX_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Mode register---------------------------------------------*/
enum
{
  MODER_INPUT   = 0,
  MODER_OUTPUT  = 1,
  MODER_AF      = 2,
  MODER_ANALOG  = 3
};

#define MODER_MODE(pin, value)          BIT_FIELD((value), (pin) * 2)
#define MODER_MODE_MASK(pin)            BIT_FIELD(MASK(2), (pin) * 2)
#define MODER_MODE_VALUE(pin, reg) \
    FIELD_VALUE((reg), MODER_MODE_MASK(pin), (pin) * 2)
/*------------------Output type register--------------------------------------*/
enum
{
  OTYPER_PP = 0,
  OTYPER_OD = 1
};
/*------------------Output speed register-------------------------------------*/
enum
{
  OSPEEDR_LOW     = 0,
  OSPEEDR_MEDIUM  = 1,
  OSPEEDR_HIGH    = 3
};

#define OSPEEDR_SPEED(pin, value)       BIT_FIELD((value), (pin) * 2)
#define OSPEEDR_SPEED_MASK(pin)         BIT_FIELD(MASK(2), (pin) * 2)
#define OSPEEDR_SPEED_VALUE(pin, reg) \
    FIELD_VALUE((reg), OSPEEDR_SPEED_MASK(pin), (pin) * 2)
/*------------------Pull-up and pull-down register----------------------------*/
enum
{
  PUPDR_NOPULL    = 0,
  PUPDR_PULLUP    = 1,
  PUPDR_PULLDOWN  = 2
};

#define PUPDR_PULL(pin, value)          BIT_FIELD((value), (pin) * 2)
#define PUPDR_PULL_MASK(pin)            BIT_FIELD(MASK(2), (pin) * 2)
#define PUPDR_PULL_VALUE(pin, reg) \
    FIELD_VALUE((reg), PUPDR_PULL_MASK(pin), (pin) * 2)
/*------------------Alternate function register-------------------------------*/
#define AFR_AFSEL(pin, value)           BIT_FIELD((value), ((pin) & 0x07) * 4)
#define AFR_AFSEL_MASK(pin)             BIT_FIELD(MASK(4), ((pin) & 0x07) * 4)
#define AFR_AFSEL_VALUE(pin, reg) \
    FIELD_VALUE((reg), AFR_AFSEL_MASK(pin), ((pin) & 0x07) * 4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_PIN_DEFS_H_ */
