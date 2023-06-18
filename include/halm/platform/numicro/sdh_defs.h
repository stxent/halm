/*
 * halm/platform/numicro/sdh_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SDH_DEFS_H_
#define HALM_PLATFORM_NUMICRO_SDH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------DMA Control and Status register---------------------------*/
#define DMACTL_DMAEN                    BIT(0)
#define DMACTL_DMARST                   BIT(1)
#define DMACTL_SGEN                     BIT(3)
#define DMACTL_DMABUSY                  BIT(9)
/*------------------DMA Transfer Starting Address register--------------------*/
#define DMASA_ORDER                     BIT(0)
#define DMASA_ADDRESS_MASK              BIT_FIELD(MASK(30), 2)
/*------------------DMA Transfer Byte Count register--------------------------*/
#define DMABCNT_BCNT_MASK               BIT_FIELD(MASK(26), 0)
/*------------------DMA Interrupt Enable Control register---------------------*/
#define DMAINTEN_ABORTIEN               BIT(0)
#define DMAINTEN_WEOTIEN                BIT(1)
/*------------------DMA Interrupt Status register-----------------------------*/
#define DMAINTSTS_ABORTIF               BIT(0)
#define DMAINTSTS_WEOTIF                BIT(1)
/*------------------Global Control and Status register------------------------*/
#define GCTL_GCTLRST                    BIT(0)
#define GCTL_SDEN                       BIT(1)
/*------------------Global Interrupt Control register-------------------------*/
#define GINTEN_DTAIEN                   BIT(0)
/*------------------Global Interrupt Status register--------------------------*/
#define GINTSTS_DTAIF                   BIT(0)
/*------------------SD Control and Status register----------------------------*/
#define CTL_COEN                        BIT(0)
#define CTL_RIEN                        BIT(1)
#define CTL_DIEN                        BIT(2)
#define CTL_DOEN                        BIT(3)
#define CTL_R2EN                        BIT(4)
#define CTL_CLK74OEN                    BIT(5)
#define CTL_CLK8OEN                     BIT(6)
#define CTL_CLKKEEP                     BIT(7)

#define CTL_CMDCODE(value)              BIT_FIELD((value), 8)
#define CTL_CMDCODE_MASK                BIT_FIELD(MASK(6), 8)
#define CTL_CMDCODE_VALUE(reg)          FIELD_VALUE((reg), CTL_CMDCODE_MASK, 8)

#define CTL_CTLRST                      BIT(14)
#define CTL_DBW                         BIT(15)

#define CTL_BLKCNT(value)               BIT_FIELD((value), 16)
#define CTL_BLKCNT_MASK                 BIT_FIELD(MASK(8), 16)
#define CTL_BLKCNT_VALUE(reg)           FIELD_VALUE((reg), CTL_BLKCNT_MASK, 16)

#define CTL_SDNWR(value)                BIT_FIELD((value), 24)
#define CTL_SDNWR_MASK                  BIT_FIELD(MASK(4), 24)
#define CTL_SDNWR_VALUE(reg)            FIELD_VALUE((reg), CTL_SDNWR_MASK, 24)
/*------------------SD Interrupt Control register-----------------------------*/
#define INTEN_BLKDIEN                   BIT(0)
#define INTEN_CRCIEN                    BIT(1)
#define INTEN_CDIEN                     BIT(8)
#define INTEN_RTOIEN                    BIT(12)
#define INTEN_DITOIEN                   BIT(13)
#define INTEN_WKIEN                     BIT(14)
#define INTEN_CDSRC                     BIT(30)
/*------------------SD Interrupt Status register------------------------------*/
enum
{
  CRCSTS_POSITIVE           = 2,
  CRCSTS_NEGATIVE           = 5,
  CRCSTS_PROGRAMMING_ERROR  = 7
};

#define INTSTS_BLKDIF                   BIT(0)
#define INTSTS_CRCIF                    BIT(1)
#define INTSTS_CRC7                     BIT(2)
#define INTSTS_CRC16                    BIT(3)

#define INTSTS_CRCSTS(value)            BIT_FIELD((value), 4)
#define INTSTS_CRCSTS_MASK              BIT_FIELD(MASK(3), 4)
#define INTSTS_CRCSTS_VALUE(reg) \
    FIELD_VALUE((reg), INTSTS_CRCSTS_MASK, 4)

#define INTSTS_DAT0STS                  BIT(7)
#define INTSTS_CDIF                     BIT(8)
#define INTSTS_RTOIF                    BIT(12)
#define INTSTS_DITOIF                   BIT(13)
#define INTSTS_CDSTS                    BIT(16)
#define INTSTS_DAT1STS                  BIT(18)
/*------------------Response/Data-in Time-out register------------------------*/
#define TOUT_TOUT_MAX                   MASK(24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SDH_DEFS_H_ */
