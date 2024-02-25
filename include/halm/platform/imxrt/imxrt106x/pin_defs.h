/*
 * halm/platform/imxrt/imxrt106x/pin_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_IMXRT106X_PIN_DEFS_H_
#define HALM_PLATFORM_IMXRT_IMXRT106X_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Interrupt Configuration Registers-------------------------*/
#define ICR_CONDITION_MASK(channel)     BIT_FIELD(MASK(3), (channel) * 2)
#define ICR_CONDITION(channel, value)   BIT_FIELD((value), (channel) * 2)
/*------------------Mux Control registers-------------------------------------*/
#define MUX_CTL_MUX_MODE(value)         BIT_FIELD((value), 0)
#define MUX_CTL_MUX_MODE_MASK           BIT_FIELD(MASK(3), 0)
#define MUX_CTL_MUX_MODE_VALUE(reg) \
    FIELD_VALUE((reg), MUX_CTL_MUX_MODE_MASK, 0)

#define MUX_CTL_SION                    BIT(4)
/*------------------Pad Control registers-------------------------------------*/
enum
{
  DSE_DISABLED = 0,
  DSE_R0       = 1,
  DSE_R0_DIV_2 = 2,
  DSE_R0_DIV_3 = 3,
  DSE_R0_DIV_4 = 4,
  DSE_R0_DIV_5 = 5,
  DSE_R0_DIV_6 = 6,
  DSE_R0_DIV_7 = 7
};

enum
{
  PUS_100K_PD = 0,
  PUS_47K_PU  = 1,
  PUS_100K_PU = 2,
  PUS_22K_PU  = 3
};

enum
{
  SPEED_LOW_50MHZ     = 0,
  SPEED_MEDIUM_100MHZ = 2,
  SPEED_HIGH_200MHZ   = 3
};

/* Enable Fast Slew Rate */
#define PAD_CTL_SRE                     BIT(0)

#define PAD_CTL_DSE(value)              BIT_FIELD((value), 3)
#define PAD_CTL_DSE_MASK                BIT_FIELD(MASK(3), 3)
#define PAD_CTL_DSE_VALUE(reg)          FIELD_VALUE((reg), PAD_CTL_DSE_MASK, 3)

#define PAD_CTL_SPEED(value)            BIT_FIELD((value), 6)
#define PAD_CTL_SPEED_MASK              BIT_FIELD(MASK(2), 6)
#define PAD_CTL_SPEED_VALUE(reg) \
    FIELD_VALUE((reg), PAD_CTL_SPEED_MASK, 6)

/* Enable Open-Drain output */
#define PAD_CTL_ODE                     BIT(11)
/* Enable Pull/Keep mode */
#define PAD_CTL_PKE                     BIT(12)
/* Enable Pull mode */
#define PAD_CTL_PUE                     BIT(13)

#define PAD_CTL_PUS(value)              BIT_FIELD((value), 14)
#define PAD_CTL_PUS_MASK                BIT_FIELD(MASK(2), 14)
#define PAD_CTL_PUS_VALUE(reg)          FIELD_VALUE((reg), PAD_CTL_PUS_MASK, 14)

/* Enable input hysteresis */
#define PAD_CTL_HYS                     BIT(16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_IMXRT106X_PIN_DEFS_H_ */
