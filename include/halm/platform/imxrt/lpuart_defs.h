/*
 * halm/platform/imxrt/lpuart_defs.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_LPUART_DEFS_H_
#define HALM_PLATFORM_IMXRT_LPUART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(function, daisy) (((daisy) << 3) | (function))
#define UNPACK_DAISY(value)         (((value) >> 3) & 0x1F)
#define UNPACK_FUNCTION(value)      ((value) & 0x07)
/*------------------Global register-------------------------------------------*/
#define GLOBAL_RST                      BIT(1)
/*------------------Pin Configuration register--------------------------------*/
enum
{
  TRGSEL_DISABLED = 0,
  TRGSEL_RXD_PIN  = 1,
  TRGSEL_CTS_PIN  = 2,
  TRGSEL_TXD_PIN  = 3
};

#define PINCFG_TRGSEL_MASK              BIT_FIELD(MASK(2), 0)
#define PINCFG_TRGSEL(value)            BIT_FIELD((value), 0)
#define PINCFG_TRGSEL_VALUE(reg) \
    FIELD_VALUE((reg), PINCFG_TRGSEL_MASK, 0)
/*------------------Baud rate register----------------------------------------*/
enum
{
  MATCFG_ADDRESS_WAKEUP   = 0,
  MATCFG_IDLE_WAKEUP      = 1,
  MATCFG_MATCH_ON_OFF     = 2,
  MATCFG_RWU_MATCH_ON_OFF = 3
};

#define BAUD_SBR_MASK                   BIT_FIELD(MASK(13), 0)
#define BAUD_SBR(value)                 BIT_FIELD((value), 0)
#define BAUD_SBR_VALUE(reg)             FIELD_VALUE((reg), BAUD_SBR_MASK, 0)
#define BAUD_SBR_MAX                    MASK(13)

#define BAUD_SBNS                       BIT(13)
#define BAUD_RXEDGIE                    BIT(14)
#define BAUD_LBKDIE                     BIT(15)
#define BAUD_RESYNCDIS                  BIT(16)
#define BAUD_BOTHEDGE                   BIT(17)

#define BAUD_MATCFG_MASK                BIT_FIELD(MASK(2), 18)
#define BAUD_MATCFG(value)              BIT_FIELD((value), 18)
#define BAUD_MATCFG_VALUE(reg)          FIELD_VALUE((reg), BAUD_MATCFG_MASK, 18)

#define BAUD_RIDMAE                     BIT(20)
#define BAUD_RDMAE                      BIT(21)
#define BAUD_TDMAE                      BIT(23)

#define BAUD_OSR_MASK                   BIT_FIELD(MASK(5), 24)
#define BAUD_OSR(value)                 BIT_FIELD((value), 24)
#define BAUD_OSR_VALUE(reg)             FIELD_VALUE((reg), BAUD_OSR_MASK, 24)

#define BAUD_M10                        BIT(29)
#define BAUD_MAEN2                      BIT(30)
#define BAUD_MAEN1                      BIT(31)
/*------------------Status register-------------------------------------------*/
#define STAT_MA2F                       BIT(14)
#define STAT_MA1F                       BIT(15)
#define STAT_PF                         BIT(16)
#define STAT_FE                         BIT(17)
#define STAT_NF                         BIT(18)
#define STAT_OR                         BIT(19)
#define STAT_IDLE                       BIT(20)
#define STAT_RDRF                       BIT(21)
#define STAT_TC                         BIT(22)
#define STAT_TDRE                       BIT(23)
#define STAT_RAF                        BIT(24)
#define STAT_LBKDE                      BIT(25)
#define STAT_BRK13                      BIT(26)
#define STAT_RWUID                      BIT(27)
#define STAT_RXINV                      BIT(28)
#define STAT_MSBF                       BIT(29)
#define STAT_RXEDGEIF                   BIT(30)
#define STAT_LBKDIF                     BIT(31)
/*------------------Control register------------------------------------------*/
enum
{
  IDLECFG_1_CHAR    = 0,
  IDLECFG_2_CHARS   = 1,
  IDLECFG_4_CHARS   = 2,
  IDLECFG_8_CHARS   = 3,
  IDLECFG_16_CHARS  = 4,
  IDLECFG_32_CHARS  = 5,
  IDLECFG_64_CHARS  = 6,
  IDLECFG_128_CHARS = 7
};

#define CTRL_PT                         BIT(0)
#define CTRL_PE                         BIT(1)
#define CTRL_ILT                        BIT(2)
#define CTRL_WAKE                       BIT(3)
#define CTRL_M                          BIT(4)
#define CTRL_RSRC                       BIT(5)
#define CTRL_DOZEEN                     BIT(6)
#define CTRL_LOOPS                      BIT(7)

#define CTRL_IDLECFG_MASK               BIT_FIELD(MASK(3), 8)
#define CTRL_IDLECFG(value)             BIT_FIELD((value), 8)
#define CTRL_IDLECFG_VALUE(reg)         FIELD_VALUE((reg), CTRL_IDLECFG_MASK, 8)

#define CTRL_M7                         BIT(11)

#define CTRL_MA2IE                      BIT(14)
#define CTRL_MA1IE                      BIT(15)
#define CTRL_SBK                        BIT(16)
#define CTRL_RWU                        BIT(17)
#define CTRL_RE                         BIT(18)
#define CTRL_TE                         BIT(19)
#define CTRL_ILIE                       BIT(20)
#define CTRL_RIE                        BIT(21)
#define CTRL_TCIE                       BIT(22)
#define CTRL_TIE                        BIT(23)
#define CTRL_PEIE                       BIT(24)
#define CTRL_FEIE                       BIT(25)
#define CTRL_NEIE                       BIT(26)
#define CTRL_ORIE                       BIT(27)
#define CTRL_TXINV                      BIT(28)
#define CTRL_TXDIR                      BIT(29)
#define CTRL_R9T8                       BIT(30)
#define CTRL_R8T9                       BIT(31)
/*------------------Data register---------------------------------------------*/
#define DATA_IDLINE                     BIT(11)
#define DATA_RXEMPT                     BIT(12)
#define DATA_FRETSC                     BIT(13)
#define DATA_PARITYE                    BIT(14)
#define DATA_NOISY                      BIT(15)
/*------------------Match register--------------------------------------------*/
#define MATCH_MA1_MASK                  BIT_FIELD(MASK(10), 0)
#define MATCH_MA1(value)                BIT_FIELD((value), 0)
#define MATCH_MA1_VALUE(reg)            FIELD_VALUE((reg), MATCH_MA1_MASK, 0)

#define MATCH_MA2_MASK                  BIT_FIELD(MASK(10), 16)
#define MATCH_MA2(value)                BIT_FIELD((value), 16)
#define MATCH_MA2_VALUE(reg)            FIELD_VALUE((reg), MATCH_MA2_MASK, 16)
/*------------------Modem IrDA Register---------------------------------------*/
#define MODIR_TXCTSE                    BIT(0)
#define MODIR_TXRTSE                    BIT(1)
#define MODIR_TXRTSPOL                  BIT(2)
#define MODIR_RXRTSE                    BIT(3)
#define MODIR_TXCTSC                    BIT(4)
#define MODIR_TXCTSSRC                  BIT(5)

#define MODIR_RTSWATER_MASK             BIT_FIELD(MASK(2), 8)
#define MODIR_RTSWATER(value)           BIT_FIELD((value), 8)
#define MODIR_RTSWATER_VALUE(reg) \
    FIELD_VALUE((reg), MODIR_RTSWATER_MASK, 8)

#define MODIR_TNP_MASK                  BIT_FIELD(MASK(2), 16)
#define MODIR_TNP(value)                BIT_FIELD((value), 16)
#define MODIR_TNP_VALUE(reg)            FIELD_VALUE((reg), MODIR_TNP_MASK, 16)

#define MODIR_IREN                      BIT(18)
/*------------------FIFO register---------------------------------------------*/
enum
{
  FIFOSIZE_1_WORD     = 0,
  FIFOSIZE_4_WORDS    = 0,
  FIFOSIZE_8_WORDS    = 0,
  FIFOSIZE_16_WORDS   = 0,
  FIFOSIZE_32_WORDS   = 0,
  FIFOSIZE_64_WORDS   = 0,
  FIFOSIZE_128_WORDS  = 0,
  FIFOSIZE_256_WORDS  = 0
};

enum
{
  RXIDEN_DISABLE  = 0,
  RXIDEN_1_CHAR   = 1,
  RXIDEN_2_CHARS  = 2,
  RXIDEN_4_CHARS  = 3,
  RXIDEN_8_CHARS  = 4,
  RXIDEN_16_CHARS = 5,
  RXIDEN_32_CHARS = 6,
  RXIDEN_64_CHARS = 7
};

#define FIFO_RXFIFOSIZE_MASK            BIT_FIELD(MASK(3), 0)
#define FIFO_RXFIFOSIZE(value)          BIT_FIELD((value), 0)
#define FIFO_RXFIFOSIZE_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_RXFIFOSIZE_MASK, 0)

#define FIFO_RXFE                       BIT(3)

#define FIFO_TXFIFOSIZE_MASK            BIT_FIELD(MASK(3), 4)
#define FIFO_TXFIFOSIZE(value)          BIT_FIELD((value), 4)
#define FIFO_TXFIFOSIZE_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_TXFIFOSIZE_MASK, 4)

#define FIFO_TXFE                       BIT(7)
#define FIFO_RXUFE                      BIT(8)
#define FIFO_TXOFE                      BIT(9)

#define FIFO_RXIDEN_MASK                BIT_FIELD(MASK(3), 10)
#define FIFO_RXIDEN(value)              BIT_FIELD((value), 10)
#define FIFO_RXIDEN_VALUE(reg)          FIELD_VALUE((reg), FIFO_RXIDEN_MASK, 10)

#define FIFO_RXFLUSH                    BIT(14)
#define FIFO_TXFLUSH                    BIT(15)
#define FIFO_RXUF                       BIT(16)
#define FIFO_TXOF                       BIT(17)
#define FIFO_RXEMPT                     BIT(22)
#define FIFO_TXEMPT                     BIT(23)
/*------------------Watermark register----------------------------------------*/
#define WATER_TXWATER_MASK              BIT_FIELD(MASK(2), 0)
#define WATER_TXWATER(value)            BIT_FIELD((value), 0)
#define WATER_TXWATER_VALUE(reg) \
    FIELD_VALUE((reg), WATER_TXWATER_MASK, 0)

#define WATER_TXCOUNT_MASK              BIT_FIELD(MASK(3), 8)
#define WATER_TXCOUNT(value)            BIT_FIELD((value), 8)
#define WATER_TXCOUNT_VALUE(reg) \
    FIELD_VALUE((reg), WATER_TXCOUNT_MASK, 8)

#define WATER_RXWATER_MASK              BIT_FIELD(MASK(2), 16)
#define WATER_RXWATER(value)            BIT_FIELD((value), 16)
#define WATER_RXWATER_VALUE(reg) \
    FIELD_VALUE((reg), WATER_RXWATER_MASK, 16)

#define WATER_RXCOUNT_MASK              BIT_FIELD(MASK(3), 24)
#define WATER_RXCOUNT(value)            BIT_FIELD((value), 24)
#define WATER_RXCOUNT_VALUE(reg) \
    FIELD_VALUE((reg), WATER_RXCOUNT_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_LPUART_DEFS_H_ */
