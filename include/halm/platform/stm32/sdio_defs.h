/*
 * halm/platform/stm32/sdio_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SDIO_DEFS_H_
#define HALM_PLATFORM_STM32_SDIO_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define FIFO_SIZE                       32
/*------------------Power control register------------------------------------*/
enum
{
  PWRCTRL_POWER_OFF = 0,
  PWRCTRL_POWER_UP  = 2, /* Reserved value */
  PWRCTRL_POWER_ON  = 3
};

#define POWER_PWRCTRL_MASK              BIT_FIELD(MASK(2), 0)
#define POWER_PWRCTRL(value)            BIT_FIELD((value), 0)
#define POWER_PWRCTRL_VALUE(reg) \
    FIELD_VALUE((reg), POWER_PWRCTRL_MASK, 0)
/*------------------Clock Control Register------------------------------------*/
enum
{
  WIDBUS_1BIT = 0,
  WIDBUS_4BIT = 1,
  WIDBUS_8BIT = 2
};

#define CLKCR_CLKDIV_MASK               BIT_FIELD(MASK(8), 0)
#define CLKCR_CLKDIV(value)             BIT_FIELD((value), 0)
#define CLKCR_CLKDIV_VALUE(reg)         FIELD_VALUE((reg), CLKCR_CLKDIV_MASK, 0)
#define CLKCR_CLKDIV_MAX                255

#define CLKCR_CLKEN                     BIT(8)
#define CLKCR_PWRSAV                    BIT(9)
#define CLKCR_BYPASS                    BIT(10)

#define CLKCR_WIDBUS_MASK               BIT_FIELD(MASK(2), 11)
#define CLKCR_WIDBUS(value)             BIT_FIELD((value), 11)
#define CLKCR_WIDBUS_VALUE(reg) \
    FIELD_VALUE((reg), CLKCR_WIDBUS_MASK, 11)

#define CLKCR_NEGEDGE                   BIT(13)
#define CLKCR_HWFC_EN                   BIT(14)
/*------------------Command register------------------------------------------*/
enum
{
  WAITRESP_NONE   = 0,
  WAITRESP_SHORT  = 1,
  WAITRESP_LONG   = 3
};

#define CMD_CMDINDEX_MASK               BIT_FIELD(MASK(6), 0)
#define CMD_CMDINDEX(value)             BIT_FIELD((value), 0)
#define CMD_CMDINDEX_VALUE(reg)         FIELD_VALUE((reg), CMD_CMDINDEX_MASK, 0)

#define CMD_WAITRESP_MASK               BIT_FIELD(MASK(2), 6)
#define CMD_WAITRESP(value)             BIT_FIELD((value), 6)
#define CMD_WAITRESP_VALUE(reg)         FIELD_VALUE((reg), CMD_WAITRESP_MASK, 6)

#define CMD_WAITINT                     BIT(8)
#define CMD_WAITPEND                    BIT(9)
#define CMD_CPSMEN                      BIT(10)
#define CMD_SDIOSuspend                 BIT(11)
#define CMD_ENCMDCompletion             BIT(12)
#define CMD_nIEN                        BIT(13)
#define CMD_ATACMD                      BIT(14)
/*------------------Data Control register-------------------------------------*/
#define DCTRL_DTEN                      BIT(0)
#define DCTRL_DTDIR                     BIT(1)
#define DCTRL_DTMODE                    BIT(2)
#define DCTRL_DMAEN                     BIT(3)

#define DCTRL_DBLOCKSIZE_MASK           BIT_FIELD(MASK(4), 4)
#define DCTRL_DBLOCKSIZE(value)         BIT_FIELD((value), 4)
#define DCTRL_DBLOCKSIZE_VALUE(reg) \
    FIELD_VALUE((reg), DCTRL_DBLOCKSIZE_MASK, 4)
#define DCTRL_DBLOCKSIZE_MAX            14

#define DCTRL_RWSTART                   BIT(8)
#define DCTRL_RWSTOP                    BIT(9)
#define DCTRL_RWMOD                     BIT(10)
#define DCTRL_SDIOEN                    BIT(11)
/*------------------Status register-------------------------------------------*/
#define STA_CCRCFAIL                    BIT(0)
#define STA_DCRCFAIL                    BIT(1)
#define STA_CTIMEOUT                    BIT(2)
#define STA_DTIMEOUT                    BIT(3)
#define STA_TXUNDERR                    BIT(4)
#define STA_RXOVERR                     BIT(5)
#define STA_CMDREND                     BIT(6)
#define STA_CMDSENT                     BIT(7)
#define STA_DATAEND                     BIT(8)
#define STA_STBITERR                    BIT(9)
#define STA_DBCKEND                     BIT(10)
#define STA_CMDACT                      BIT(11)
#define STA_TXACT                       BIT(12)
#define STA_RXACT                       BIT(13)
#define STA_TXFIFOHE                    BIT(14)
#define STA_RXFIFOHF                    BIT(15)
#define STA_TXFIFOF                     BIT(16)
#define STA_RXFIFOF                     BIT(17)
#define STA_TXFIFOE                     BIT(18)
#define STA_RXFIFOE                     BIT(19)
#define STA_TXDAVL                      BIT(20)
#define STA_RXDAVL                      BIT(21)
#define STA_SDIOIT                      BIT(22)
#define STA_CEATAEND                    BIT(23)
/*------------------Interrupt Clear Register----------------------------------*/
#define ICR_CCRCFAILC                   BIT(0)
#define ICR_DCRCFAILC                   BIT(1)
#define ICR_CTIMEOUTC                   BIT(2)
#define ICR_DTIMEOUTC                   BIT(3)
#define ICR_TXUNDERRC                   BIT(4)
#define ICR_RXOVERRC                    BIT(5)
#define ICR_CMDRENDC                    BIT(6)
#define ICR_CMDSENTC                    BIT(7)
#define ICR_DATAENDC                    BIT(8)
#define ICR_STBITERRC                   BIT(9)
#define ICR_DBCKENDC                    BIT(10)
#define ICR_SDIOITC                     BIT(22)
#define ICR_CEATAENDC                   BIT(23)

#define ICR_MASK                        MASK(24)
/*------------------Mask register---------------------------------------------*/
#define MASK_CCRCFAILIE                 BIT(0)
#define MASK_DCRCFAILIE                 BIT(1)
#define MASK_CTIMEOUTIE                 BIT(2)
#define MASK_DTIMEOUTIE                 BIT(3)
#define MASK_TXUNDERRIE                 BIT(4)
#define MASK_RXOVERRIE                  BIT(5)
#define MASK_CMDRENDIE                  BIT(6)
#define MASK_CMDSENTIE                  BIT(7)
#define MASK_DATAENDIE                  BIT(8)
#define MASK_STBITERRIE                 BIT(9)
#define MASK_DBCKENDIE                  BIT(10)
#define MASK_CMDACTIE                   BIT(11)
#define MASK_TXACTIE                    BIT(12)
#define MASK_RXACTIE                    BIT(13)
#define MASK_TXFIFOHEIE                 BIT(14)
#define MASK_RXFIFOHFIE                 BIT(15)
#define MASK_TXFIFOFIE                  BIT(16)
#define MASK_RXFIFOFIE                  BIT(17)
#define MASK_TXFIFOEIE                  BIT(18)
#define MASK_RXFIFOEIE                  BIT(19)
#define MASK_TXDAVLIE                   BIT(20)
#define MASK_RXDAVLIE                   BIT(21)
#define MASK_SDIOITIE                   BIT(22)
#define MASK_CEATAENDIE                 BIT(23)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SDIO_DEFS_H_ */
