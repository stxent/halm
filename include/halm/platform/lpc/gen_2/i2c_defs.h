/*
 * halm/platform/lpc/gen_2/i2c_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_I2C_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register------------------------------------*/
#define CFG_MSTEN                       BIT(0)
#define CFG_SLVEN                       BIT(1)
#define CFG_MONEN                       BIT(2)
#define CFG_TIMEOUTEN                   BIT(3)
#define CFG_MONCLKSTR                   BIT(4)
/*------------------Status register-------------------------------------------*/
enum
{
  MSTSTATE_IDLE         = 0,
  MSTSTATE_RX_READY     = 1,
  MSTSTATE_TX_READY     = 2,
  MSTSTATE_NACK_ADDRESS = 3,
  MSTSTATE_NACK_DATA    = 4
};

enum
{
  SLVSTATE_ADDRESS      = 0,
  SLVSTATE_RX_READY     = 1,
  SLVSTATE_TX_READY     = 2
};

#define STAT_MSTPENDING                 BIT(0)

#define STAT_MSTSTATE(value)            BIT_FIELD((value), 1)
#define STAT_MSTSTATE_MASK              BIT_FIELD(MASK(3), 1)
#define STAT_MSTSTATE_VALUE(reg) \
    FIELD_VALUE((reg), STAT_MSTSTATE_MASK, 1)

#define STAT_MSTARBLOSS                 BIT(4)
#define STAT_MSTSTSTPERR                BIT(6)
#define STAT_SLVPENDING                 BIT(8)

#define STAT_SLVSTATE(value)            BIT_FIELD((value), 9)
#define STAT_SLVSTATE_MASK              BIT_FIELD(MASK(2), 9)
#define STAT_SLVSTATE_VALUE(reg) \
    FIELD_VALUE((reg), STAT_SLVSTATE_MASK, 9)

#define STAT_SLVNOTSTR                  BIT(11)

#define STAT_SLVIDX(value)              BIT_FIELD((value), 12)
#define STAT_SLVIDX_MASK                BIT_FIELD(MASK(2), 12)
#define STAT_SLVIDX_VALUE(reg) \
    FIELD_VALUE((reg), STAT_SLVIDX_MASK, 12)

#define STAT_SLVSEL                     BIT(14)
#define STAT_SLVDESEL                   BIT(15)
#define STAT_MONRDY                     BIT(16)
#define STAT_MONOV                      BIT(17)
#define STAT_MONACTIVE                  BIT(18)
#define STAT_MONIDLE                    BIT(19)
#define STAT_EVENTTIMEOUT               BIT(24)
#define STAT_SCLTIMEOUT                 BIT(25)

#define STAT_MST_MASK \
    (STAT_MSTARBLOSS | STAT_MSTSTSTPERR | STAT_EVENTTIMEOUT | STAT_SCLTIMEOUT)
#define STAT_MSK_ERROR_MASK             STAT_MST_MASK
/*------------------Interrupt Enable Set and read register--------------------*/
#define INTENSET_MSTPENDINGEN           BIT(0)
#define INTENSET_MSTARBLOSSEN           BIT(4)
#define INTENSET_MSTSTSTPERREN          BIT(6)
#define INTENSET_SLVPENDINGEN           BIT(8)
#define INTENSET_SLVNOTSTREN            BIT(11)
#define INTENSET_SLVDESELEN             BIT(15)
#define INTENSET_MONRDYEN               BIT(16)
#define INTENSET_MONOVEN                BIT(17)
#define INTENSET_MONIDLEEN              BIT(19)
#define INTENSET_EVENTTIMEOUTEN         BIT(24)
#define INTENSET_SCLTIMEOUTEN           BIT(25)
/*------------------Interrupt Enable Clear register---------------------------*/
#define INTENCLR_MSTPENDINGCLR          BIT(0)
#define INTENCLR_MSTARBLOSSCLR          BIT(4)
#define INTENCLR_MSTSTSTPERRCLR         BIT(6)
#define INTENCLR_SLVPENDINGCLR          BIT(8)
#define INTENCLR_SLVNOTSTRCLR           BIT(11)
#define INTENCLR_SLVDESELCLR            BIT(15)
#define INTENCLR_MONRDYCLR              BIT(16)
#define INTENCLR_MONOVCLR               BIT(17)
#define INTENCLR_MONIDLECLR             BIT(19)
#define INTENCLR_EVENTTIMEOUTCLR        BIT(24)
#define INTENCLR_SCLTIMEOUTCLR          BIT(25)
/*------------------Time-out value register-----------------------------------*/
#define TIMEOUT_TOMIN(value)            BIT_FIELD((value), 0)
#define TIMEOUT_TOMIN_MASK              BIT_FIELD(MASK(4), 0)
#define TIMEOUT_TOMIN_VALUE(reg) \
    FIELD_VALUE((reg), TIMEOUT_TOMIN_MASK, 0)

#define TIMEOUT_TO(value)               BIT_FIELD((value), 4)
#define TIMEOUT_TO_MASK                 BIT_FIELD(MASK(12), 4)
#define TIMEOUT_TO_VALUE(reg)           FIELD_VALUE((reg), TIMEOUT_TO_MASK, 4)
/*------------------Clock Divider register------------------------------------*/
#define CLKDIV_MAX                      65535
/*------------------Interrupt Status register---------------------------------*/
#define INTSTAT_MSTPENDING              BIT(0)
#define INTSTAT_MSTARBLOSS              BIT(4)
#define INTSTAT_MSTSTSTPERR             BIT(6)
#define INTSTAT_SLVPENDING              BIT(8)
#define INTSTAT_SLVNOTSTR               BIT(11)
#define INTSTAT_SLVDESEL                BIT(15)
#define INTSTAT_MONRDY                  BIT(16)
#define INTSTAT_MONOV                   BIT(17)
#define INTSTAT_MONIDLE                 BIT(19)
#define INTSTAT_EVENTTIMEOUT            BIT(24)
#define INTSTAT_SCLTIMEOUT              BIT(25)
/*------------------Master Control register-----------------------------------*/
#define MSTCTL_MSTCONTINUE              BIT(0)
#define MSTCTL_MSTSTART                 BIT(1)
#define MSTCTL_MSTSTOP                  BIT(2)
#define MSTCTL_MSTDMA                   BIT(3)
/*------------------Master Time register--------------------------------------*/
#define MSTTIME_MSTSCLLOW(value)        BIT_FIELD((value), 0)
#define MSTTIME_MSTSCLLOW_MASK          BIT_FIELD(MASK(3), 0)
#define MSTTIME_MSTSCLLOW_VALUE(reg) \
    FIELD_VALUE((reg), MSTTIME_MSTSCLLOW_MASK, 0)

#define MSTTIME_MSTSCLHIGH(value)       BIT_FIELD((value), 4)
#define MSTTIME_MSTSCLHIGH_MASK         BIT_FIELD(MASK(3), 4)
#define MSTTIME_MSTSCLHIGH_VALUE(reg) \
    FIELD_VALUE((reg), MSTTIME_MSTSCLHIGH_MASK, 4)
/*------------------Master Data register--------------------------------------*/
#define MSTDAT_READ                     1
#define MSTDAT_WRITE                    0
/*------------------Slave Control register------------------------------------*/
#define SLVCTL_SLVCONTINUE              BIT(0)
#define SLVCTL_SLVNACK                  BIT(1)
#define SLVCTL_SLVDMA                   BIT(3)
/*------------------Slave Address register------------------------------------*/
#define SLVADR_SADISABLE                BIT(0)

#define SLVADR_SLVADR(value)            BIT_FIELD((value), 1)
#define SLVADR_SLVADR_MASK              BIT_FIELD(MASK(7), 1)
#define SLVADR_SLVADR_VALUE(reg) \
    FIELD_VALUE((reg), SLVADR_SLVADR_MASK, 1)
/*------------------Slave address Qualifier 0 register------------------------*/
#define SLVQUAL0_QUALMODE0              BIT(0)

#define SLVQUAL0_SLVQUAL0(value)        BIT_FIELD((value), 1)
#define SLVQUAL0_SLVQUAL0_MASK          BIT_FIELD(MASK(7), 1)
#define SLVQUAL0_SLVQUAL0_VALUE(reg) \
    FIELD_VALUE((reg), SLVQUAL0_SLVQUAL0_MASK, 1)
/*------------------Monitor Data register-------------------------------------*/
#define MONRXDAT_MONRXDAT_MASK          BIT_FIELD(MASK(8), 0)
#define MONRXDAT_MONRXDAT_VALUE(reg) \
    FIELD_VALUE((reg), MONRXDAT_MONRXDAT_MASK, 0)

#define MONRXDAT_MONSTART               BIT(8)
#define MONRXDAT_MONRESTART             BIT(9)
#define MONRXDAT_MONNACK                BIT(10)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_I2C_DEFS_H_ */
