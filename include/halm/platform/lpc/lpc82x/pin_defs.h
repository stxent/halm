/*
 * halm/platform/lpc/lpc82x/pin_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC82X_PIN_DEFS_H_
#define HALM_PLATFORM_LPC_LPC82X_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------IO Configuration registers--------------------------------*/
#define IOCON_I2C_MASK                  BIT_FIELD(MASK(2), 8)
/* Standard or Fast-mode with input glitch filter */
#define IOCON_I2C_STANDARD              BIT_FIELD(0, 8)
/* Open-drain input-output without glitch filter */
#define IOCON_I2C_IO                    BIT_FIELD(1, 8)
/* Fast-mode Plus with input glitch filter */
#define IOCON_I2C_PLUS                  BIT_FIELD(2, 8)
/*----------------------------------------------------------------------------*/
#define IOCON_MODE_MASK                 BIT_FIELD(MASK(2), 3)
#define IOCON_MODE_INACTIVE             BIT_FIELD(0, 3)
#define IOCON_MODE_PULLDOWN             BIT_FIELD(1, 3)
#define IOCON_MODE_PULLUP               BIT_FIELD(2, 3)
#define IOCON_MODE_REPEATER             BIT_FIELD(3, 3)
/*----------------------------------------------------------------------------*/
#define IOCON_HYS                       BIT(5)
#define IOCON_INV                       BIT(6)
#define IOCON_OD                        BIT(10)
/*----------------------------------------------------------------------------*/
enum
{
  S_MODE_BYPASS   = 0,
  S_MODE_1_CYCLE  = 1,
  S_MODE_2_CYCLES = 2,
  S_MODE_3_CYCLES = 3
};

#define IOCON_S_MODE_MASK               BIT_FIELD(MASK(2), 11)
#define IOCON_S_MODE(value)             BIT_FIELD((value), 11)
#define IOCON_S_MODE_VALUE(reg) \
    FIELD_VALUE((reg), IOCON_S_MODE_MASK, 11)
/*----------------------------------------------------------------------------*/
#define IOCON_CLK_DIV_MASK              BIT_FIELD(MASK(3), 13)
#define IOCON_CLK_DIV(value)            BIT_FIELD((value), 13)
#define IOCON_CLK_DIV_VALUE(reg) \
    FIELD_VALUE((reg), IOCON_CLK_DIV_MASK, 13)
/*----------------------------------------------------------------------------*/
#define PINASSIGN_MUX_MASK(offset)      BIT_FIELD(MASK(8), (offset) << 3)
#define PINASSIGN_MUX(offset, value)    BIT_FIELD((value), (offset) << 3)
#define PINASSIGN_MUX_VALUE(offset, reg) \
    FIELD_VALUE((reg), PINASSIGN_MUX_MASK, (offset) << 3)
/*----------------------------------------------------------------------------*/
#define PINTSEL_CHANNEL_MASK(channel)   BIT_FIELD(MASK(8), (channel) << 3)
#define PINTSEL_CHANNEL(channel, port, number) \
    BIT_FIELD(((port) << 5) | (number), (channel) << 3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_PIN_DEFS_H_ */
