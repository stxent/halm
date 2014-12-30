/*
 * platform/nxp/gptimer_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GPTIMER_DEFS_H_
#define PLATFORM_NXP_GPTIMER_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <bits.h>
/*------------------Interrupt Register----------------------------------------*/
#define IR_MATCH_INTERRUPT(channel)     BIT((channel))
#define IR_CAPTURE_INTERRUPT(channel)   BIT((channel) + 4)
/*------------------Timer Control Register------------------------------------*/
#define TCR_CEN                         BIT(0) /* Enable for counting */
#define TCR_CRES                        BIT(1) /* Synchronous reset */
/*------------------Match Control Register------------------------------------*/
#define MCR_INTERRUPT(channel)          BIT((channel) * 3)
#define MCR_RESET(channel)              BIT((channel) * 3 + 1)
#define MCR_STOP(channel)               BIT((channel) * 3 + 2)
/*------------------Capture Control Register----------------------------------*/
#define CCR_MASK(channel)               BIT_FIELD(MASK(3), (channel) * 3)
#define CCR_RISING_EDGE(channel)        BIT((channel) * 3)
#define CCR_FALLING_EDGE(channel)       BIT((channel) * 3 + 1)
#define CCR_INTERRUPT(channel)          BIT((channel) * 3 + 2)
/*------------------Count Control Register------------------------------------*/
enum
{
  MODE_TIMER    = 0,
  MODE_RISING   = 1,
  MODE_FALLING  = 2,
  MODE_TOGGLE   = 3
};

#define CTCR_MODE_MASK                  BIT_FIELD(MASK(2), 0)
#define CTCR_MODE(value)                BIT_FIELD((value), 0)
#define CTCR_MODE_VALUE(reg)            FIELD_VALUE((reg), CTCR_MODE_MASK, 0)
#define CTCR_INPUT_MASK                 BIT_FIELD(MASK(2), 2)
#define CTCR_INPUT(channel)             BIT_FIELD((channel), 2)
#define CTCR_INPUT_VALUE(reg)           FIELD_VALUE((reg), CTCR_INPUT_MASK, 2)
/*------------------External Match Register-----------------------------------*/
enum
{
  CONTROL_CLEAR   = 1,
  CONTROL_SET     = 2,
  CONTROL_TOGGLE  = 3
};

#define EMR_CONTROL_MASK(channel)       BIT_FIELD(MASK(2), (channel) * 2 + 4)
#define EMR_CONTROL(channel, value)     BIT_FIELD((value), (channel) * 2 + 4)
#define EMR_EXTERNAL_MATCH(channel)     BIT((channel))
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GPTIMER_DEFS_H_ */
