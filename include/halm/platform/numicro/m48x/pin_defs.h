/*
 * halm/platform/numicro/m48x/pin_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M48X_PIN_DEFS_H_
#define HALM_PLATFORM_NUMICRO_M48X_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------I/O Mode Control------------------------------------------*/
enum
{
  MODE_INPUT                = 0,
  MODE_PUSH_PULL            = 1,
  MODE_OPEN_DRAIN           = 2,
  MODE_QUASI_BIDIRECTIONAL  = 3
};

#define MODE_MODE_MASK(pin)             BIT_FIELD(MASK(2), (pin) * 2)
#define MODE_MODE(pin, value)           BIT_FIELD((value), (pin) * 2)
/*------------------Digital Input Path Disable Control------------------------*/
#define DINOFF_OFFSET                   16
/*------------------Interrupt Enable Control register-------------------------*/
#define INTEN_FLIEN(pin)                BIT((pin) + 0)
#define INTEN_FLIEN_GROUP(value)        BIT_FIELD((value), 0)
#define INTEN_RHIEN(pin)                BIT((pin) + 16)
#define INTEN_RHIEN_GROUP(value)        BIT_FIELD((value), 16)
/*------------------High Slew Rate Control register---------------------------*/
enum
{
  /* Maximum 40 MHz at 2.7V */
  HSREN_NORMAL  = 0,
  /* Maximum 80 MHz at 2.7V */
  HSREN_HIGH    = 1,
  /* Maximum 100 MHz at 2.7V */
  HSREN_FAST    = 2
};

#define SLEWCTL_HSREN_MASK(pin)         BIT_FIELD(MASK(2), (pin) * 2)
#define SLEWCTL_HSREN(pin, value)       BIT_FIELD((value), (pin) * 2)
/*------------------Pull-up and Pull-down Selection register------------------*/
enum
{
  PUSEL_NONE  = 0,
  PUSEL_UP    = 1,
  PUSEL_DOWN  = 2
};

#define PUSEL_PUSEL_MASK(pin)           BIT_FIELD(MASK(2), (pin) * 2)
#define PUSEL_PUSEL(pin, value)         BIT_FIELD((value), (pin) * 2)
/*------------------Interrupt De-bounce Control register----------------------*/
enum
{
  DBCLKSEL_1     = 0,
  DBCLKSEL_2     = 1,
  DBCLKSEL_4     = 2,
  DBCLKSEL_8     = 3,
  DBCLKSEL_16    = 4,
  DBCLKSEL_32    = 5,
  DBCLKSEL_64    = 6,
  DBCLKSEL_128   = 7,
  DBCLKSEL_256   = 8,
  DBCLKSEL_512   = 9,
  DBCLKSEL_1024  = 10,
  DBCLKSEL_2048  = 11,
  DBCLKSEL_4096  = 12,
  DBCLKSEL_8192  = 13,
  DBCLKSEL_16384 = 14,
  DBCLKSEL_32768 = 15
};

#define DBCTL_DBCLKSEL(value)           BIT_FIELD((value), 0)
#define DBCTL_DBCLKSEL_MASK             BIT_FIELD(MASK(4), 0)
#define DBCTL_DBCLKSEL_VALUE(reg) \
    FIELD_VALUE((reg), DBCTL_DBCLKSEL_MASK, 0)

#define DBCTL_DBCLKSRC                  BIT(4)
#define DBCTL_ICLKON                    BIT(5)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_PIN_DEFS_H_ */
