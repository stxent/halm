/*
 * halm/platform/numicro/spim_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SPIM_DEFS_H_
#define HALM_PLATFORM_NUMICRO_SPIM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define SPIM_MAX_TRANSFER_SIZE          ((1 << 24) - 1)
/*------------------Control and Status register 0-----------------------------*/
enum
{
  DWIDTH_8  = 0x07,
  DWIDTH_16 = 0x0F,
  DWIDTH_24 = 0x17,
  DWIDTH_32 = 0x1F
};

enum
{
  BITMODE_STANDARD,
  BITMODE_DUAL,
  BITMODE_QUAD
};

enum
{
  OPMODE_NORMAL,
  OPMODE_DMA_WRITE,
  OPMODE_DMA_READ,
  OPMODE_DMM
};

#define CTL0_CIPHOFF                    BIT(0)
#define CTL0_BALEN                      BIT(2)
#define CTL0_B4ADDREN                   BIT(5)
#define CTL0_IEN                        BIT(6)
#define CTL0_IF                         BIT(7)

#define CTL0_DWIDTH_MASK                BIT_FIELD(MASK(5), 8)
#define CTL0_DWIDTH(value)              BIT_FIELD((value), 8)
#define CTL0_DWIDTH_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_DWIDTH_MASK, 8)

#define CTL0_BURSTNUM_MASK              BIT_FIELD(MASK(2), 13)
#define CTL0_BURSTNUM(value)            BIT_FIELD((value), 13)
#define CTL0_BURSTNUM_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_BURSTNUM_MASK, 13)

#define CTL0_QDIODIR                    BIT(15)

#define CTL0_SUSPITV_MASK               BIT_FIELD(MASK(4), 16)
#define CTL0_SUSPITV(value)             BIT_FIELD((value), 16)
#define CTL0_SUSPITV_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_SUSPITV_MASK, 16)

#define CTL0_BITMODE_MASK               BIT_FIELD(MASK(2), 20)
#define CTL0_BITMODE(value)             BIT_FIELD((value), 20)
#define CTL0_BITMODE_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_BITMODE_MASK, 20)

#define CTL0_OPMODE_MASK                BIT_FIELD(MASK(2), 22)
#define CTL0_OPMODE(value)              BIT_FIELD((value), 22)
#define CTL0_OPMODE_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_OPMODE_MASK, 22)

#define CTL0_CMDCODE_MASK               BIT_FIELD(MASK(8), 24)
#define CTL0_CMDCODE(value)             BIT_FIELD((value), 24)
#define CTL0_CMDCODE_VALUE(reg) \
    FIELD_VALUE((reg), CTL0_CMDCODE_MASK, 24)
/*------------------Control register 1----------------------------------------*/
#define CTL1_SPIMEN                     BIT(0)
#define CTL1_CACHEOFF                   BIT(1)
#define CTL1_CCMEN                      BIT(2)
#define CTL1_CDINVAL                    BIT(3)
#define CTL1_SS                         BIT(4)
#define CTL1_SSACTPOL                   BIT(5)

#define CTL1_IDLETIME_MAX               15
#define CTL1_IDLETIME_MASK              BIT_FIELD(MASK(4), 8)
#define CTL1_IDLETIME(value)            BIT_FIELD((value), 8)
#define CTL1_IDLETIME_VALUE(reg) \
    FIELD_VALUE((reg), CTL1_IDLETIME_MASK, 8)

#define CTL1_DIVIDER_MAX                65534
#define CTL1_DIVIDER_MASK               BIT_FIELD(MASK(16), 16)
#define CTL1_DIVIDER(value)             BIT_FIELD((value), 16)
#define CTL1_DIVIDER_VALUE(reg) \
    FIELD_VALUE((reg), CTL1_DIVIDER_MASK, 16)
/*------------------RX Clock Delay Control register---------------------------*/
#define RXCLKDLY_DWDELSEL_MASK          BIT_FIELD(MASK(8), 0)
#define RXCLKDLY_DWDELSEL(value)        BIT_FIELD((value), 0)
#define RXCLKDLY_DWDELSEL_VALUE(reg) \
    FIELD_VALUE((reg), RXCLKDLY_DWDELSEL_MASK, 0)

#define RXCLKDLY_PHDELSEL_MASK          BIT_FIELD(MASK(8), 8)
#define RXCLKDLY_PHDELSEL(value)        BIT_FIELD((value), 8)
#define RXCLKDLY_PHDELSEL_VALUE(reg) \
    FIELD_VALUE((reg), RXCLKDLY_PHDELSEL_MASK, 8)

#define RXCLKDLY_RDDLYSEL_MASK          BIT_FIELD(MASK(3), 16)
#define RXCLKDLY_RDDLYSEL(value)        BIT_FIELD((value), 16)
#define RXCLKDLY_RDDLYSEL_VALUE(reg) \
    FIELD_VALUE((reg), RXCLKDLY_RDDLYSEL_MASK, 16)

#define RXCLKDLY_RDEDGE                 BIT(20)
/*------------------Direct Memory Mapping Mode Control register---------------*/
#define DMMCTL_CRMDAT_MASK              BIT_FIELD(MASK(8), 8)
#define DMMCTL_CRMDAT(value)            BIT_FIELD((value), 8)
#define DMMCTL_CRMDAT_VALUE(reg) \
    FIELD_VALUE((reg), DMMCTL_CRMDAT_MASK, 8)

#define DMMCTL_DESELTIM_MAX             31
#define DMMCTL_DESELTIM_MASK            BIT_FIELD(MASK(5), 16)
#define DMMCTL_DESELTIM(value)          BIT_FIELD((value), 16)
#define DMMCTL_DESELTIM_VALUE(reg) \
    FIELD_VALUE((reg), DMMCTL_DESELTIM_MASK, 16)

#define DMMCTL_BWEN                     BIT(24)
#define DMMCTL_CREN                     BIT(25)
#define DMMCTL_UACTSCLK                 BIT(26)

#define DMMCTL_ACTSCLKT_MASK            BIT_FIELD(MASK(4), 28)
#define DMMCTL_ACTSCLKT(value)          BIT_FIELD((value), 28)
#define DMMCTL_ACTSCLKT_VALUE(reg) \
    FIELD_VALUE((reg), DMMCTL_ACTSCLKT_MASK, 28)
/*------------------Control register 2----------------------------------------*/
#define CTL2_USETEN                     BIT(16)
#define CTL2_DTRMPOFF                   BIT(20)

#define CTL2_DCNUM_MAX                  31
#define CTL2_DCNUM_MASK                 BIT_FIELD(MASK(5), 24)
#define CTL2_DCNUM(value)               BIT_FIELD((value), 24)
#define CTL2_DCNUM_VALUE(reg)           FIELD_VALUE((reg), CTL2_DCNUM_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SPIM_DEFS_H_ */
