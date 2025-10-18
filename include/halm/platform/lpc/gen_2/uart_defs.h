/*
 * halm/platform/lpc/gen_2/uart_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_UART_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register------------------------------------*/
enum
{
  DATALEN_7BIT  = 0,
  DATALEN_8BIT  = 1,
  DATALEN_9BIT  = 2
};

enum
{
  PARITYSEL_NONE  = 0,
  PARITYSEL_EVEN  = 2,
  PARITYSEL_ODD   = 3
};

#define CFG_ENABLE                      BIT(0)

#define CFG_DATALEN(value)              BIT_FIELD((value), 2)
#define CFG_DATALEN_MASK                BIT_FIELD(MASK(2), 2)
#define CFG_DATALEN_VALUE(reg)          FIELD_VALUE((reg), CFG_DATALEN_MASK, 2)

#define CFG_PARITYSEL(value)            BIT_FIELD((value), 4)
#define CFG_PARITYSEL_MASK              BIT_FIELD(MASK(2), 4)
#define CFG_PARITYSEL_VALUE(reg) \
    FIELD_VALUE((reg), CFG_PARITYSEL_MASK, 4)

#define CFG_STOPLEN                     BIT(6)
#define CFG_CTSEN                       BIT(9)
#define CFG_SYNCEN                      BIT(11)
#define CFG_CLKPOL                      BIT(12)
#define CFG_SYNCMST                     BIT(14)
#define CFG_LOOP                        BIT(15)
#define CFG_OETA                        BIT(18)
#define CFG_AUTOADDR                    BIT(19)
#define CFG_OESEL                       BIT(20)
#define CFG_OEPOL                       BIT(21)
#define CFG_RXPOL                       BIT(22)
#define CFG_TXPOL                       BIT(23)
/*------------------Control register------------------------------------------*/
#define CTL_TXBRKEN                     BIT(1)
#define CTL_ADDRDET                     BIT(2)
#define CTL_TXDIS                       BIT(6)
#define CTL_CC                          BIT(8)
#define CTL_CLRCCONRX                   BIT(9)
#define CTL_AUTOBAUD                    BIT(16)
/*------------------Status register-------------------------------------------*/
#define STAT_RXRDY                      BIT(0)
#define STAT_RXIDLE                     BIT(1)
#define STAT_TXRDY                      BIT(2)
#define STAT_TXIDLE                     BIT(3)
#define STAT_CTS                        BIT(4)
#define STAT_DELTACTS                   BIT(5)
#define STAT_TXDISSTAT                  BIT(6)
#define STAT_OVERRUNINT                 BIT(8)
#define STAT_RXBRK                      BIT(10)
#define STAT_DELTARXBRK                 BIT(11)
#define STAT_START                      BIT(12)
#define STAT_FRAMERRINT                 BIT(13)
#define STAT_PARITYERRINT               BIT(14)
#define STAT_RXNOISEINT                 BIT(15)
#define STAT_ABERR                      BIT(16)
/*------------------Interrupt Enable read and set register--------------------*/
#define INTENSET_RXRDYEN                BIT(0)
#define INTENSET_TXRDYEN                BIT(2)
#define INTENSET_TXIDLEEN               BIT(3)
#define INTENSET_DELTACTSEN             BIT(5)
#define INTENSET_TXDISEN                BIT(6)
#define INTENSET_OVERRUNEN              BIT(8)
#define INTENSET_DELTARXBRKEN           BIT(11)
#define INTENSET_STARTEN                BIT(12)
#define INTENSET_FRAMERREN              BIT(13)
#define INTENSET_PARITYERREN            BIT(14)
#define INTENSET_RXNOISEEN              BIT(15)
#define INTENSET_ABERREN                BIT(16)
/*------------------Interrupt Enable clear register---------------------------*/
#define INTENCLR_RXRDYCLR               BIT(0)
#define INTENCLR_TXRDYCLR               BIT(2)
#define INTENCLR_TXIDLECLR              BIT(3)
#define INTENCLR_DELTACTSCLR            BIT(5)
#define INTENCLR_TXDISCLR               BIT(6)
#define INTENCLR_OVERRUNCLR             BIT(8)
#define INTENCLR_DELTARXBRKCLR          BIT(11)
#define INTENCLR_STARTCLR               BIT(12)
#define INTENCLR_FRAMERRCLR             BIT(13)
#define INTENCLR_PARITYERRCLR           BIT(14)
#define INTENCLR_RXNOISECLR             BIT(15)
#define INTENCLR_ABERRCLR               BIT(16)
/*------------------Receiver Data with Status register------------------------*/
#define RXDATSTAT_RXDAT(value)          BIT_FIELD((value), 0)
#define RXDATSTAT_RXDAT_MASK            BIT_FIELD(MASK(9), 0)
#define RXDATSTAT_RXDAT_VALUE(reg) \
    FIELD_VALUE((reg), RXDATSTAT_RXDAT_MASK, 0)

#define RXDATSTAT_FRAMERR               BIT(13)
#define RXDATSTAT_PARITYERR             BIT(14)
#define RXDATSTAT_RXNOISE               BIT(15)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTAT_RXRDY                   BIT(0)
#define INTSTAT_TXRDY                   BIT(2)
#define INTSTAT_TXIDLE                  BIT(3)
#define INTSTAT_DELTACTS                BIT(5)
#define INTSTAT_TXDISINT                BIT(6)
#define INTSTAT_OVERRUNINT              BIT(8)
#define INTSTAT_DELTARXBRKINT           BIT(11)
#define INTSTAT_START                   BIT(12)
#define INTSTAT_FRAMERRINT              BIT(13)
#define INTSTAT_PARITYERRINT            BIT(14)
#define INTSTAT_RXNOISEINT              BIT(15)
#define INTSTAT_ABERR                   BIT(16)
/*------------------Oversample Selection Register-----------------------------*/
#define OSR_OSRVAL(value)               BIT_FIELD((value), 0)
#define OSR_OSRVAL_MASK                 BIT_FIELD(MASK(4), 0)
#define OSR_OSRVAL_VALUE(reg)           FIELD_VALUE((reg), OSR_OSRVAL_MASK, 0)
#define OSR_OSRVAL_MIN                  4
#define OSR_OSRVAL_MAX                  15
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_UART_DEFS_H_ */
