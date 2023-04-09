/*
 * halm/platform/lpc/lpc43xx/atimer_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_ATIMER_DEFS_H_
#define HALM_PLATFORM_LPC_LPC43XX_ATIMER_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Preset value register-------------------------------------*/
#define PRESET_MAX                      MASK(16)
/*------------------Interrupt Clear Enable register---------------------------*/
#define CLR_EN_CLR_EN                   BIT(0)
/*------------------Interrupt Set Enable register-----------------------------*/
#define SET_EN_SET_EN                   BIT(0)
/*------------------Interrupt Status register---------------------------------*/
#define STATUS_STAT                     BIT(0)
/*------------------Interrupt Enable register---------------------------------*/
#define ENABLE_EN                       BIT(0)
/*------------------Clear Status register-------------------------------------*/
#define CLR_STAT_CSTAT                  BIT(0)
/*------------------Set Status register---------------------------------------*/
#define SET_STAT_SSTAT                  BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ATIMER_DEFS_H_ */
