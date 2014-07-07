/*
 * platform/nxp/lpc11exx/system_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SYSTEM_DEFS_H_
#define SYSTEM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Peripheral Reset Control register-------------------------*/
#define PRESETCTRL_SSP0                 BIT(0)
#define PRESETCTRL_I2C                  BIT(1)
#define PRESETCTRL_SSP1                 BIT(2)
/*------------------Power Control register------------------------------------*/
#define PCON_PM_MASK                    BIT_FIELD(MASK(3), 0)
#define PCON_PM(value)                  BIT_FIELD((value), 0)
#define PCON_PM_VALUE(reg)              FIELD_VALUE((reg), PCON_PM_MASK, 0)
#define PCON_PM_DEFAULT                 0
#define PCON_PM_DEEPSLEEP               1
#define PCON_PM_POWERDOWN               2
#define PCON_PM_DEEPPOWERDOWN           3

#define PCON_PM_NODPD                   BIT(3)
#define PCON_PM_SLEEPFLAG               BIT(8)
#define PCON_PM_DPDFLAG                 BIT(11)
/*----------------------------------------------------------------------------*/
#endif /* SYSTEM_DEFS_H_ */
