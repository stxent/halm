/*
 * halm/platform/stm32/iwdg_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_IWDG_DEFS_H_
#define HALM_PLATFORM_STM32_IWDG_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Key register----------------------------------------------*/
#define KR_RELOAD                       0xAAAA
#define KR_START                        0xCCCC
#define KR_UNLOCK                       0x5555
/*------------------Reload register-------------------------------------------*/
#define RLR_RL_MAX                      MASK(12)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_IWDG_DEFS_H_ */
