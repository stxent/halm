/*
 * halm/platform/lpc/gptimer_pwm_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPTIMER_PWM_DEFS_H_
#define HALM_PLATFORM_LPC_GPTIMER_PWM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gptimer_defs.h>
/*------------------Interrupt Register----------------------------------------*/
#define IR_PWM_INTERRUPT(channel)       BIT((channel) + 8)
/*------------------PWM Control Register--------------------------------------*/
#define PWMC_ENABLE(channel)            BIT(channel)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPTIMER_PWM_DEFS_H_ */
