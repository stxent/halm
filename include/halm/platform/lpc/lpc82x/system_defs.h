/*
 * halm/platform/lpc/lpc82x/system_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC82X_SYSTEM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC82X_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Flash Configuration register------------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(2), 0)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 0)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 0)
/*------------------Power Control register------------------------------------*/
enum
{
  PCON_PM_DEFAULT       = 0,
  PCON_PM_DEEPSLEEP     = 1,
  PCON_PM_POWERDOWN     = 2,
  PCON_PM_DEEPPOWERDOWN = 3
};

#define PCON_PM_MASK                    BIT_FIELD(MASK(3), 0)
#define PCON_PM(value)                  BIT_FIELD((value), 0)
#define PCON_PM_VALUE(reg)              FIELD_VALUE((reg), PCON_PM_MASK, 0)

#define PCON_NODPD                      BIT(3)
#define PCON_SLEEPFLAG                  BIT(8)
#define PCON_DPDFLAG                    BIT(11)
/*------------------Interrupt wake-up enable registers------------------------*/
enum
{
  STARTERP1_SPI0    = 0,
  STARTERP1_SPI1    = 1,
  STARTERP1_USART0  = 3,
  STARTERP1_USART1  = 4,
  STARTERP1_USART2  = 5,
  STARTERP1_I2C1    = 7,
  STARTERP1_I2C0    = 8,
  STARTERP1_WWDT    = 12,
  STARTERP1_BOD     = 13,
  STARTERP1_WKT     = 15,
  STARTERP1_I2C2    = 21,
  STARTERP1_I2C3    = 22
};

#define STARTERP0_PINT(channel)         BIT(channel)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_SYSTEM_DEFS_H_ */
