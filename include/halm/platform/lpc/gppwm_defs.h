/*
 * halm/platform/lpc/gppwm_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GPPWM_DEFS_H_
#define HALM_PLATFORM_LPC_GPPWM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/gptimer_defs.h>
/*------------------Interrupt Register----------------------------------------*/
#define PWM_IR_MATCH_INTERRUPT(channel) \
    ((channel) < 4 ? BIT(channel) : BIT((channel) + 4))
#define PWM_IR_MATCH_MASK \
    (BIT_FIELD(MASK(4), 0) | BIT_FIELD(MASK(3), 8))
#define PWM_IR_MATCH_VALUE(reg) \
    FIELD_VALUE((reg), PWM_IR_MATCH_MASK, 0)
/*------------------Timer Control Register------------------------------------*/
#define TCR_PWM_ENABLE                  BIT(3)
/*------------------PWM Control Register--------------------------------------*/
/* Available for channels from 2 to 6 */
#define PCR_DOUBLE_EDGE(channel)        BIT(channel)
/* Available for channels from 1 to 6 */
#define PCR_OUTPUT_ENABLED(channel)     BIT((channel) + 8)
/*------------------PWM Latch Enable Register---------------------------------*/
#define LER_ENABLE(channel)             BIT(channel)
#define LER_MASK                        MASK(7)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GPPWM_DEFS_H_ */
