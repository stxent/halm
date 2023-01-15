/*
 * halm/platform/numicro/pdma_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_PDMA_DEFS_H_
#define HALM_PLATFORM_NUMICRO_PDMA_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Descriptor Table Control register-------------------------*/
enum
{
  ADDRESS_INC = 0,
  ADDRESS_FIX = 3
};

enum
{
  OPMODE_IDLE   = 0,
  OPMODE_BASIC  = 1,
  OPMODE_LIST   = 2,
  OPMODE_ACTIVE = 3
};

#define DSCT_CTL_OPMODE(value)          BIT_FIELD((value), 0)
#define DSCT_CTL_OPMODE_MASK            BIT_FIELD(MASK(2), 0)
#define DSCT_CTL_OPMODE_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_OPMODE_MASK, 0)

#define DSCT_CTL_TXTYPE                 BIT(2)

#define DSCT_CTL_BURSIZE(value)         BIT_FIELD((value), 4)
#define DSCT_CTL_BURSIZE_MASK           BIT_FIELD(MASK(3), 4)
#define DSCT_CTL_BURSIZE_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_BURSIZE_MASK, 4)

#define DSCT_CTL_TBINTDIS               BIT(7)

#define DSCT_CTL_SAINC(value)           BIT_FIELD((value), 8)
#define DSCT_CTL_SAINC_MASK             BIT_FIELD(MASK(2), 8)
#define DSCT_CTL_SAINC_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_SAINC_MASK, 8)

#define DSCT_CTL_DAINC(value)           BIT_FIELD((value), 10)
#define DSCT_CTL_DAINC_MASK             BIT_FIELD(MASK(2), 10)
#define DSCT_CTL_DAINC_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_DAINC_MASK, 10)

#define DSCT_CTL_TXWIDTH(value)         BIT_FIELD((value), 12)
#define DSCT_CTL_TXWIDTH_MASK           BIT_FIELD(MASK(2), 12)
#define DSCT_CTL_TXWIDTH_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_TXWIDTH_MASK, 12)

#define DSCT_CTL_TXCNT(value)           BIT_FIELD((value), 16)
#define DSCT_CTL_TXCNT_MASK             BIT_FIELD(MASK(16), 16)
#define DSCT_CTL_TXCNT_VALUE(reg) \
    FIELD_VALUE((reg), DSCT_CTL_TXCNT_MASK, 16)
/*------------------Next SC Descriptor Table Offset Address-------------------*/
#define DSCT_NEXT_ADDRESS(value)        ((value) & MASK(16))

#define DSCT_NEXT_NEXT_MASK             BIT_FIELD(MASK(16), 0)
#define DSCT_NEXT_ADDRESS_TO_NEXT(value) \
    ((value) & DSCT_NEXT_NEXT_MASK)

#define DSCT_NEXT_EXENEXT_MASK          BIT_FIELD(MASK(16), 16)
#define DSCT_NEXT_EXENEXT_TO_ADDRESS(reg) \
    FIELD_VALUE((reg), DSCT_NEXT_EXENEXT_MASK, 16)
/*------------------Channel Control register----------------------------------*/
#define CHCTL_CH(channel)               BIT(channel)
#define CHCTL_CH_MASK \
    BIT_FIELD(MASK(CONFIG_PLATFORM_NUMICRO_PDMA_COUNT), 0)
/*------------------Transfer Pause Control register---------------------------*/
#define PAUSE_CH(channel)               BIT(channel)
#define PAUSE_CH_MASK \
    BIT_FIELD(MASK(CONFIG_PLATFORM_NUMICRO_PDMA_COUNT), 0)
/*------------------Software Request register---------------------------------*/
#define SWREQ_CH(channel)               BIT(channel)
/*------------------Channel Request Status register---------------------------*/
#define TRGSTS_CH(channel)              BIT(channel)
/*------------------Fixed Priority Set register-------------------------------*/
#define PRISET_CH(channel)              BIT(channel)
/*------------------Fixed Priority Clear register-----------------------------*/
#define PRICLR_CH(channel)              BIT(channel)
/*------------------Interrupt Enable register---------------------------------*/
#define INTEN_CH(channel)               BIT(channel)
#define INTEN_CH_MASK \
    BIT_FIELD(MASK(CONFIG_PLATFORM_NUMICRO_PDMA_COUNT), 0)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTS_ABTIF                    BIT(0)
#define INTSTS_TDIF                     BIT(1)
#define INTSTS_ALIGNF                   BIT(2)
#define INTSTS_REQTOF0                  BIT(8)
#define INTSTS_REQTOF1                  BIT(9)

#define INTSTS_REQTOF(channel)          BIT((channel) + 8)
#define INTSTS_REQTOF_MASK              BIT_FIELD(MASK(2), 8)
#define INTSTS_REQTOF_VALUE(reg) \
    FIELD_VALUE((reg), INTSTS_REQTOF_MASK, 0)
/*------------------Channel Read/Write Target Abort Flag register-------------*/
#define ABTSTS_CH(channel)              BIT(channel)
/*------------------Channel Transfer Done Flag register-----------------------*/
#define TDSTS_CH(channel)               BIT(channel)
/*------------------Transfer Alignment Status register------------------------*/
#define ALIGN_CH(channel)               BIT(channel)
/*------------------Transfer Active Flag register-----------------------------*/
#define TACTSTS_CH(channel)             BIT(channel)
/*------------------Time-out Prescaler register-------------------------------*/
enum
{
  TOUTPSC_HCLK_DIV256,
  TOUTPSC_HCLK_DIV512,
  TOUTPSC_HCLK_DIV1024,
  TOUTPSC_HCLK_DIV2048,
  TOUTPSC_HCLK_DIV4096,
  TOUTPSC_HCLK_DIV8192,
  TOUTPSC_HCLK_DIV16384,
  TOUTPSC_HCLK_DIV32768
};

#define TOUTPSC_CH_0(value)             BIT_FIELD((value), 0)
#define TOUTPSC_CH_0_MASK               BIT_FIELD(MASK(3), 0)
#define TOUTPSC_CH_0_VALUE(reg)         FIELD_VALUE((reg), TOUTPSC_CH_0_MASK, 0)

#define TOUTPSC_CH_1(value)             BIT_FIELD((value), 4)
#define TOUTPSC_CH_1_MASK               BIT_FIELD(MASK(3), 4)
#define TOUTPSC_CH_1_VALUE(reg)         FIELD_VALUE((reg), TOUTPSC_CH_1_MASK, 4)

#define TOUTPSC_CH(channel, value)      BIT_FIELD((value), (channel) * 4)
#define TOUTPSC_CH_MASK(channel)        BIT_FIELD(MASK(3), (channel) * 4)
#define TOUTPSC_CH_VALUE(channel, reg) \
    FIELD_VALUE((reg), TOUTPSC_CH_MASK(channel), (channel) * 4)
/*------------------Time-out Enable register----------------------------------*/
#define TOUTEN_CH_0                     BIT(0)
#define TOUTEN_CH_1                     BIT(1)
#define TOUTEN_CH(channel)              BIT(channel)
/*------------------Time-out Interrupt Enable register------------------------*/
#define TOUTIEN_CH_0                    BIT(0)
#define TOUTIEN_CH_1                    BIT(1)
#define TOUTIEN_CH(channel)             BIT(channel)
/*------------------SC Descriptor Table Base Address register-----------------*/
#define SCATBA_MASK                     BIT_FIELD(MASK(16), 16)
#define SCATBA_ADDRESS_TO_BASE(value)   ((value) & SCATBA_MASK)
#define SCATBA_BASE_TO_ADDRESS(reg)     ((reg) & SCATBA_MASK)
/*------------------Time-out Period Counter register--------------------------*/
#define TOC_CH_0(value)                 BIT_FIELD((value), 0)
#define TOC_CH_0_MASK                   BIT_FIELD(MASK(16), 0)
#define TOC_CH_0_VALUE(reg)             FIELD_VALUE((reg), TOC_CH_0_MASK, 0)

#define TOC_CH_1(value)                 BIT_FIELD((value), 0)
#define TOC_CH_1_MASK                   BIT_FIELD(MASK(16), 0)
#define TOC_CH_1_VALUE(reg)             FIELD_VALUE((reg), TOC_CH_1_MASK, 0)

#define TOC_CH(channel, value)          BIT_FIELD((value), (channel) * 16)
#define TOC_CH_MASK(channel)            BIT_FIELD(MASK(16), (channel) * 16)
#define TOC_CH_VALUE(channel, reg) \
    FIELD_VALUE((reg), TOC_CH_MASK(channel), (channel) * 16)
/*------------------Channel Reset register------------------------------------*/
#define CHRST_CH(channel)               BIT(channel)
#define CHRST_CH_MASK \
    BIT_FIELD(MASK(CONFIG_PLATFORM_NUMICRO_PDMA_COUNT), 0)
/*------------------Request Source Select registers---------------------------*/
#define REQSEL_CH(channel, value)       BIT_FIELD((value), (channel) * 8)
#define REQSEL_CH_MASK(channel)         BIT_FIELD(MASK(6), (channel) * 8)
#define REQSEL_CH_VALUE(channel, reg) \
    FIELD_VALUE((reg), REQSEL_CH_MASK(channel), (channel) * 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_PDMA_DEFS_H_ */
