/*
 * halm/platform/stm32/exti_defs.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_EXTI_DEFS_H_
#define HALM_PLATFORM_STM32_EXTI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Interrupt Mask Register-----------------------------------*/
#define IMR_IM(ch)                      BIT(ch)
/*------------------Event Mask Register---------------------------------------*/
#define EMR_EM(ch)                      BIT(ch)
/*------------------Rising Trigger Selection Register-------------------------*/
#define RTSR_RT(ch)                     BIT(ch)
/*------------------Falling Trigger Selection Register------------------------*/
#define FTSR_FT(ch)                     BIT(ch)
/*------------------Software Interrupt Event Register-------------------------*/
#define SWIER_SWI(ch)                   BIT(ch)
/*------------------Pending Register------------------------------------------*/
#define PR_PIF(ch)                      BIT(ch)
#define PR_PIF_MASK(begin, end)         (MASK(begin) & ~MASK(end))
/*------------------External Interrupt Configuration Registers----------------*/
#define EXTICR_INPUT(offset, value)     BIT_FIELD((value), (offset) << 2)
#define EXTICR_INPUT_MASK(offset)       BIT_FIELD(MASK(4), (offset) << 2)
#define EXTICR_INPUT_VALUE(reg, offset) (((reg) >> ((offset) << 2) & MASK(4))
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_EXTI_DEFS_H_ */
