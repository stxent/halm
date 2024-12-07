/*
 * halm/platform/lpc/sct_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SCT_DEFS_H_
#define HALM_PLATFORM_LPC_SCT_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register------------------------------------*/
enum
{
  CLKMODE_BUS       = 0,
  /* High-performance sampled-clock mode, SCT clock is from the bus clock */
  CLKMODE_INPUT_HP  = 1,
  /* Low-performance sampled-clock mode, SCT clock is from the clock input */
  CLKMODE_INPUT_LP  = 2
};

#define CONFIG_UNIFY                    BIT(0)
#define CONFIG_CLKMODE_MASK             BIT_FIELD(MASK(2), 1)
#define CONFIG_CLKMODE(value)           BIT_FIELD((value), 1)
#define CONFIG_CKSEL_MASK               BIT_FIELD(MASK(4), 3)
#define CONFIG_CKSEL_RISING(channel)    BIT_FIELD((channel) << 1, 3)
#define CONFIG_CKSEL_FALLING(channel)   BIT_FIELD(((channel) << 1) + 1, 3)
#define CONFIG_NORELOAD(part)           BIT((part) + 7)
#define CONFIG_INSYNC_MASK              BIT_FIELD(MASK(4), 9)
#define CONFIG_INSYNC(channel)          BIT_FIELD(BIT(channel), 9)
#define CONFIG_AUTOLIMIT(part)          BIT((part) + 17)

#define CONFIG_SHARED_MASK \
    (CONFIG_UNIFY | CONFIG_CLKMODE_MASK | CONFIG_CKSEL_MASK)
/*------------------Control register------------------------------------------*/
#define CTRL_DOWN                       BIT(0)
#define CTRL_STOP                       BIT(1)
#define CTRL_HALT                       BIT(2)
#define CTRL_CLRCTR                     BIT(3)
#define CTRL_BIDIR                      BIT(4)
#define CTRL_PRE_MASK                   BIT_FIELD(MASK(8), 5)
#define CTRL_PRE(value)                 BIT_FIELD((value), 5)
#define CTRL_PRE_VALUE(reg)             FIELD_VALUE((reg), CTRL_PRE_MASK, 5)
/*------------------State register--------------------------------------------*/
#define STATE_MASK                      BIT_FIELD(MASK(5), 0)
#define STATE_VALUE(reg)                FIELD_VALUE((reg), STATE_MASK, 0)
/*------------------Input register--------------------------------------------*/
#define INPUT_AIN(channel)              BIT(channel)
#define INPUT_SIN(channel)              BIT((channel) + 16)
/*------------------Bidirectional output control register---------------------*/
enum
{
  SETCLR_INDEPENDENT  = 0,
  /* Reverse set and clear for Unified or Low part counting down */
  SETCLR_L_REVERSE    = 1,
  /* Reverse set and clear for High part counting down */
  SETCLR_H_REVERSE    = 2
};

#define OUTPUTDIRCTRL_SETCLR_MASK(channel) \
    BIT_FIELD(MASK(2), (channel) << 1)
#define OUTPUTDIRCTRL_SETCLR(channel, value) \
    BIT_FIELD((value), (channel) << 1)
/*------------------Conflict resolution register------------------------------*/
enum
{
  OUTPUT_NO_CHANGE  = 0,
  OUTPUT_SET        = 1,
  OUTPUT_CLEAR      = 2,
  OUTPUT_TOGGLE     = 3
};

#define RES_OUTPUT_MASK(channel)        BIT_FIELD(MASK(2), (channel) << 1)
#define RES_OUTPUT(channel, value)      BIT_FIELD((value), (channel) << 1)
/*------------------DMA request registers-------------------------------------*/
#define DMAREQ_DEV_MASK                 BIT_FIELD(MASK(16), 0)
#define DMAREQ_DEV(channel)             BIT_FIELD(BIT(channel), 0)
#define DMAREQ_DRL                      BIT(30)
#define DMAREQ_DRQ                      BIT(31) /* State of request */
/*------------------Flag enable register--------------------------------------*/
#define EVEN_IEN(channel)               BIT(channel)
/*------------------Event Flag register---------------------------------------*/
#define EVFLAG_FLAG_MASK                MASK(16)
#define EVFLAG_FLAG(channel)            BIT(channel)
/*------------------Conflict Enable register----------------------------------*/
#define CONEN_NCEN_MASK                 MASK(16)
#define CONEN_NCEN(channel)             BIT(channel)
/*------------------Conflict Flag register------------------------------------*/
#define CONFLAG_NCFLAG_MASK             BIT_FIELD(MASK(16), 0)
#define CONFLAG_NCFLAG(channel)         BIT_FIELD(BIT(channel), 0)
#define CONFLAG_BUSERRL                 BIT(30)
#define CONFLAG_BUSERRH                 BIT(31)
/*------------------Event Control registers-----------------------------------*/
enum
{
  IOCOND_LOW  = 0,
  IOCOND_RISE = 1,
  IOCOND_FALL = 2,
  IOCOND_HIGH = 3
};

enum
{
  COMBMODE_OR     = 0,
  COMBMODE_MATCH  = 1,
  COMBMODE_IO     = 2,
  COMBMODE_AND    = 3
};

enum
{
  DIRECTION_INDEPENDENT = 0,
  DIRECTION_UP          = 1,
  DIRECTION_DOWN        = 2
};

#define EVCTRL_MATCHSEL_MASK            BIT_FIELD(MASK(4), 0)
#define EVCTRL_MATCHSEL(value)          BIT_FIELD((value), 0)
/* Select L counter when cleared and select H counter when set */
#define EVCTRL_HEVENT                   BIT(4)
/* Select the input when cleared and select the output when set */
#define EVCTRL_OUTSEL                   BIT(5)
#define EVCTRL_OUTSEL_IN                0
#define EVCTRL_OUTSEL_OUT               EVCTRL_OUTSEL
#define EVCTRL_IOSEL_MASK               BIT_FIELD(MASK(4), 6)
#define EVCTRL_IOSEL(value)             BIT_FIELD((value), 6)
#define EVCTRL_IOCOND_MASK              BIT_FIELD(MASK(2), 10)
#define EVCTRL_IOCOND(value)            BIT_FIELD((value), 10)
#define EVCTRL_COMBMODE_MASK            BIT_FIELD(MASK(2), 12)
#define EVCTRL_COMBMODE(value)          BIT_FIELD((value), 12)
#define EVCTRL_STATELD                  BIT(14)
#define EVCTRL_STATEV_MASK              BIT_FIELD(MASK(5), 15)
#define EVCTRL_STATEV(value)            BIT_FIELD((value), 15)
/* Enable GE or LE instead of EQ, available on flash-based parts only */
#define EVCTRL_MATCHMEM                 BIT(20)
/* Direction qualifier for BIDIR mode, available on flash-based parts only */
#define EVCTRL_DIRECTION_MASK           BIT_FIELD(MASK(2), 21)
#define EVCTRL_DIRECTION(value)         BIT_FIELD((value), 21)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SCT_DEFS_H_ */
