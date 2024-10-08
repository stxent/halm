/*
 * halm/platform/lpc/gen_3/pin_defs.h
 * Copyright (C) 2014, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_3_PIN_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_3_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------IO Configuration registers--------------------------------*/
#define IOCON_FUNC_MASK                 BIT_FIELD(MASK(3), 0)
#define IOCON_FUNC(func)                BIT_FIELD((func), 0)
#define IOCON_FUNC_VALUE(reg)           FIELD_VALUE((reg), IOCON_FUNC_MASK, 0)
/*----------------------------------------------------------------------------*/
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
#define IOCON_DIGITAL                   BIT(7)
#define IOCON_FILTR                     BIT(8)
#define IOCON_OD                        BIT(10)
/*----------------------------------------------------------------------------*/
#define PINTSEL_CHANNEL_MASK(channel)   BIT_FIELD(MASK(8), (channel) << 3)
#define PINTSEL_CHANNEL(channel, port, number) \
    BIT_FIELD(((port) << 5) | (number), (channel) << 3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_3_PIN_DEFS_H_ */
