/*
 * halm/platform/imxrt/pit_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_PIT_DEFS_H_
#define HALM_PLATFORM_IMXRT_PIT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Module Control Register-----------------------------------*/
/* Freeze */
#define MCR_FRZ                         BIT(0)
/* Module Disable for PIT */
#define MCR_MDIS                        BIT(1)
/*------------------Timer Control registers-----------------------------------*/
/* Timer Enable */
#define TCTRL_TEN                       BIT(0)
/* Timer Interrupt Enable */
#define TCTRL_TIE                       BIT(1)
/* Chain Mode */
#define TCTRL_CHN                       BIT(2)
/*------------------Timer Flag registers--------------------------------------*/
/* Timer Interrupt Flag */
#define TFLG_TIF                        BIT(0)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_PIT_DEFS_H_ */
