/*
 * halm/platform/stm/stm32f1xx/pin_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_PIN_DEFS_H_
#define HALM_PLATFORM_STM_STM32F1XX_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration Registers-----------------------------------*/
enum
{
  CNF_INPUT_ANALOG    = 0,
  CNF_INPUT_FLOATING  = 1,
  CNF_INPUT_PUPD      = 2
};

enum
{
  CNF_OUTPUT_GP     = 0x00, /* General-Purpose output */
  CNF_OUTPUT_AF     = 0x02, /* Alternate function output */

  CNF_OUTPUT_PP     = 0x00, /* Push-pull */
  CNF_OUTPUT_OD     = 0x01, /* Open-drain */

  CNF_FUNCTION_MASK = 0x02,
  CNF_OUTPUT_MASK   = 0x01
};

enum
{
  MODE_INPUT            = 0,
  MODE_OUTPUT_SPEED_10  = 1,
  MODE_OUTPUT_SPEED_2   = 2,
  MODE_OUTPUT_SPEED_50  = 3
};

#define CR_MASK(pin)                    BIT_FIELD(MASK(4), ((pin) & 0x07) * 4)

#define CR_CNF_OFFSET(pin)              (((pin) & 0x07) * 4 + 2)
#define CR_CNF(pin, value)              BIT_FIELD((value), CR_CNF_OFFSET(pin))
#define CR_CNF_MASK(pin)                BIT_FIELD(MASK(2), CR_CNF_OFFSET(pin))
#define CR_CNF_VALUE(pin, reg) \
    FIELD_VALUE((reg), CR_CNF_MASK(pin), CR_CNF_OFFSET(pin))

#define CR_MODE_OFFSET(pin)             (((pin) & 0x07) * 4)
#define CR_MODE(pin, value)             BIT_FIELD((value), CR_MODE_OFFSET(pin))
#define CR_MODE_MASK(pin)               BIT_FIELD(MASK(2), CR_MODE_OFFSET(pin))
#define CR_MODE_VALUE(pin, reg) \
    FIELD_VALUE((reg), CR_MODE_MASK(pin), CR_MODE_OFFSET(pin))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_PIN_DEFS_H_ */
