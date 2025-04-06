/*
 * halm/platform/lpc/lpc13uxx/system_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Flash Configuration register------------------------------*/
#define FLASHCFG_FLASHTIM_MASK          BIT_FIELD(MASK(2), 0)
#define FLASHCFG_FLASHTIM(value)        BIT_FIELD((value), 0)
#define FLASHCFG_FLASHTIM_VALUE(reg) \
    FIELD_VALUE((reg), FLASHCFG_FLASHTIM_MASK, 0)
/*------------------Peripheral Reset Control register-------------------------*/
#define PRESETCTRL_SSP0                 BIT(0)
#define PRESETCTRL_I2C                  BIT(1)
#define PRESETCTRL_SSP1                 BIT(2)
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
  STARTERP1_WWDT      = 12,
  STARTERP1_BOD       = 13,
  STARTERP1_USB       = 19,
  STARTERP1_GPIOINT0  = 20,
  STARTERP1_GPIOINT1  = 21
};

#define STARTERP0_PINT(channel)         BIT(channel)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_SYSTEM_DEFS_H_ */
