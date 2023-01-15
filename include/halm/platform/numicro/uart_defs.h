/*
 * halm/platform/numicro/uart_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_UART_DEFS_H_
#define HALM_PLATFORM_NUMICRO_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Interrupt Enable register---------------------------------*/
/* Receive Data Available Interrupt Enable */
#define INTEN_RDAIEN                    BIT(0)
/* Transmit Holding Register Empty Interrupt Enable */
#define INTEN_THREIEN                   BIT(1)
/* Receive Line Status Interrupt Enable */
#define INTEN_RLSIEN                    BIT(2)
/* Modem Status Interrupt Enable */
#define INTEN_MODEMIEN                  BIT(3)
/* RX Time-out Interrupt Enable */
#define INTEN_RXTOIEN                   BIT(4)
/* Buffer Error Interrupt Enable */
#define INTEN_BUFFERIEN                 BIT(5)
/* Wake-up Interrupt Enable */
#define INTEN_WKIEN                     BIT(6)
/* Receive Buffer Time-out Counter Enable */
#define INTEN_TOCNTEN                   BIT(11)
/* nRTS Auto-flow Control Enable */
#define INTEN_ATORTSEN                  BIT(12)
/* nCTS Auto-flow Control Enable */
#define INTEN_ATOCTSEN                  BIT(13)
/* TX PDMA Enable */
#define INTEN_TXPDMAEN                  BIT(14)
/* RX PDMA Enable */
#define INTEN_RXPDMAEN                  BIT(15)
/* Single-wire Bit Error Detection Interrupt Enable */
#define INTEN_SWBEIEN                   BIT(16)
/* Auto-baud Rate Interrupt Enable */
#define INTEN_ABRIEN                    BIT(18)
/* Transmitter Empty Interrupt Enable */
#define INTEN_TXENDIEN                  BIT(22)
/*------------------FIFO Control register-------------------------------------*/
enum
{
  RX_TRIGGER_LEVEL_1  = 0,
  RX_TRIGGER_LEVEL_4  = 1,
  RX_TRIGGER_LEVEL_8  = 2,
  RX_TRIGGER_LEVEL_14 = 3
};

enum
{
  RTS_TRIGGER_LEVEL_1   = 0,
  RTS_TRIGGER_LEVEL_4   = 1,
  RTS_TRIGGER_LEVEL_8   = 2,
  RTS_TRIGGER_LEVEL_14  = 3
};

#define FIFO_RXRST                      BIT(2)
#define FIFO_TXRST                      BIT(3)

#define FIFO_RFITL(value)               BIT_FIELD((value), 4)
#define FIFO_RFITL_MASK                 BIT_FIELD(MASK(4), 4)
#define FIFO_RFITL_VALUE(reg)           FIELD_VALUE((reg), FIFO_RFITL_MASK, 4)

#define FIFO_RXOFF                      BIT(8)

#define FIFO_RTSTRGLV(value)            BIT_FIELD((value), 16)
#define FIFO_RTSTRGLV_MASK              BIT_FIELD(MASK(4), 16)
#define FIFO_RTSTRGLV_VALUE(reg) \
    FIELD_VALUE((reg), FIFO_RTSTRGLV_MASK, 16)
/*------------------Line Control register-------------------------------------*/
enum
{
  WLS_5BIT = 0,
  WLS_6BIT = 1,
  WLS_7BIT = 2,
  WLS_8BIT = 3
};

#define LINE_WLS(value)                 BIT_FIELD((value), 0)
#define LINE_WLS_MASK                   BIT_FIELD(MASK(2), 0)
#define LINE_WLS_VALUE(reg)             FIELD_VALUE((reg), LINE_WLS_MASK, 0)

#define LINE_NSB                        BIT(2)
#define LINE_PBE                        BIT(3)
#define LINE_EPE                        BIT(4)
#define LINE_SPE                        BIT(5)
#define LINE_BCB                        BIT(6)
#define LINE_PSS                        BIT(7)
#define LINE_TXDINV                     BIT(8)
#define LINE_RXDINV                     BIT(9)
/*------------------Modem Control register------------------------------------*/
#define MODEM_RTS                       BIT(1)
#define MODEM_RTSACTLV                  BIT(9)
#define MODEM_RTSSTS                    BIT(13)
/*------------------Modem Status register-------------------------------------*/
#define MODEMSTS_CTSDETF                BIT(0)
#define MODEMSTS_CTSSTS                 BIT(4)
#define MODEMSTS_CTSACTLV               BIT(8)
/*------------------FIFO Status register--------------------------------------*/
#define FIFOSTS_RXOVIF                  BIT(0)
#define FIFOSTS_ABRDIF                  BIT(1)
#define FIFOSTS_ABRDTOIF                BIT(2)
#define FIFOSTS_ADDRDETF                BIT(3)
#define FIFOSTS_PEF                     BIT(4)
#define FIFOSTS_FEF                     BIT(5)
#define FIFOSTS_BIF                     BIT(6)

#define FIFOSTS_RXPTR(value)            BIT_FIELD((value), 8)
#define FIFOSTS_RXPTR_MASK              BIT_FIELD(MASK(6), 8)
#define FIFOSTS_RXPTR_VALUE(reg) \
    FIELD_VALUE((reg), FIFOSTS_RXPTR_MASK, 8)

#define FIFOSTS_RXEMPTY                 BIT(14)
#define FIFOSTS_RXFULL                  BIT(15)

#define FIFOSTS_TXPTR(value)            BIT_FIELD((value), 16)
#define FIFOSTS_TXPTR_MASK              BIT_FIELD(MASK(2), 16)
#define FIFOSTS_TXPTR_VALUE(reg) \
    FIELD_VALUE((reg), FIFOSTS_TXPTR_MASK, 16)

#define FIFOSTS_TXEMPTY                 BIT(22)
#define FIFOSTS_TXFULL                  BIT(23)
#define FIFOSTS_TXOVIF                  BIT(24)
#define FIFOSTS_TXEMPTYF                BIT(28)
#define FIFOSTS_RXIDLE                  BIT(29)
#define FIFOSTS_TXRXACT                 BIT(31)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTS_RDAIF                    BIT(0)
#define INTSTS_THREIF                   BIT(1)
#define INTSTS_RLSIF                    BIT(2)
#define INTSTS_MODEMIF                  BIT(3)
#define INTSTS_RXTOIF                   BIT(4)
#define INTSTS_BUFFERIF                 BIT(5)
#define INTSTS_WKIF                     BIT(6)
#define INTSTS_RDAINT                   BIT(8)
#define INTSTS_THREINT                  BIT(9)
#define INTSTS_RLSINT                   BIT(10)
#define INTSTS_MODEMINT                 BIT(11)
#define INTSTS_RXTOINT                  BIT(12)
#define INTSTS_BUFFERINT                BIT(13)
#define INTSTS_WKINT                    BIT(14)
#define INTSTS_SWBEIF                   BIT(16)
#define INTSTS_HWRLSIF                  BIT(18)
#define INTSTS_HWMODIF                  BIT(19)
#define INTSTS_HWTOIF                   BIT(20)
#define INTSTS_HWBUFEIF                 BIT(21)
#define INTSTS_TXENDIF                  BIT(22)
#define INTSTS_SWBEINT                  BIT(24)
#define INTSTS_HWRLSINT                 BIT(26)
#define INTSTS_HWMODINT                 BIT(27)
#define INTSTS_HWTOINT                  BIT(28)
#define INTSTS_HWBUFEINT                BIT(29)
#define INTSTS_TXENDINT                 BIT(30)
#define INTSTS_ABRINT                   BIT(31)
/*------------------Time-out register-----------------------------------------*/
#define TOUT_TOIC(value)                BIT_FIELD((value), 0)
#define TOUT_TOIC_MASK                  BIT_FIELD(MASK(8), 0)
#define TOUT_TOIC_VALUE(reg)            FIELD_VALUE((reg), TOUT_TOIC_MASK, 0)

#define TOUT_DLY(value)                 BIT_FIELD((value), 8)
#define TOUT_DLY_MASK                   BIT_FIELD(MASK(8), 8)
#define TOUT_DLY_VALUE(reg)             FIELD_VALUE((reg), TOUT_DLY_MASK, 8)
/*------------------Baud Rate Divider register--------------------------------*/
enum
{
  BAUDM_MODE_0 = 0,
  BAUDM_MODE_1 = 2,
  BAUDM_MODE_2 = 3
};

#define BAUD_BRD(value)                 BIT_FIELD((value), 0)
#define BAUD_BRD_MASK                   BIT_FIELD(MASK(16), 0)
#define BAUD_BRD_VALUE(reg)             FIELD_VALUE((reg), BAUD_BRD_MASK, 0)

#define BAUD_EDIVM1(value)              BIT_FIELD((value), 24)
#define BAUD_EDIVM1_MASK                BIT_FIELD(MASK(4), 24)
#define BAUD_EDIVM1_VALUE(reg)          FIELD_VALUE((reg), BAUD_EDIVM1_MASK, 24)

#define BAUD_BAUDM(value)               BIT_FIELD((value), 28)
#define BAUD_BAUDM_MASK                 BIT_FIELD(MASK(2), 28)
#define BAUD_BAUDM_VALUE(reg)           FIELD_VALUE((reg), BAUD_BAUDM_MASK, 28)
#define BAUD_BAUDM0                     BIT(28)
#define BAUD_BAUDM1                     BIT(29)
/*------------------IrDA Control register-------------------------------------*/
#define IRDA_TXEN                       BIT(1)
#define IRDA_TXINV                      BIT(5)
#define IRDA_RXINV                      BIT(6)
/*------------------Alternate Control/Status register-------------------------*/
#define ALTCTL_BRKFL(value)             BIT_FIELD((value), 0)
#define ALTCTL_BRKFL_MASK               BIT_FIELD(MASK(4), 0)
#define ALTCTL_BRKFL_VALUE(reg)         FIELD_VALUE((reg), ALTCTL_BRKFL_MASK, 0)

#define ALTCTL_LINRXEN                  BIT(6)
#define ALTCTL_LINTXEN                  BIT(7)
#define ALTCTL_RS485NMM                 BIT(8)
#define ALTCTL_RS485AAD                 BIT(9)
#define ALTCTL_RS485AUD                 BIT(10)
#define ALTCTL_ADDRDEN                  BIT(15)
#define ALTCTL_ABRIF                    BIT(17)
#define ALTCTL_ABRDEN                   BIT(18)

#define ALTCTL_ABRDBIT(value)           BIT_FIELD((value), 19)
#define ALTCTL_ABRDBIT_MASK             BIT_FIELD(MASK(2), 19)
#define ALTCTL_ABRDBIT_VALUE(reg) \
    FIELD_VALUE((reg), ALTCTL_ABRDBIT_MASK, 19)

#define ALTCTL_ADDRMV(value)            BIT_FIELD((value), 24)
#define ALTCTL_ADDRMV_MASK              BIT_FIELD(MASK(8), 24)
#define ALTCTL_ADDRMV_VALUE(reg) \
    FIELD_VALUE((reg), ALTCTL_ADDRMV_MASK, 24)
/*------------------Function Select register----------------------------------*/
enum
{
  FUNCSEL_UART        = 0,
  FUNCSEL_IRDA        = 1,
  FUNCSEL_RS485       = 2,
  FUNCSEL_SINGLE_WIRE = 3
};

#define FUNCSEL_FUNCSEL(value)          BIT_FIELD((value), 0)
#define FUNCSEL_FUNCSEL_MASK            BIT_FIELD(MASK(2), 0)
#define FUNCSEL_FUNCSEL_VALUE(reg) \
    FIELD_VALUE((reg), FUNCSEL_FUNCSEL_MASK, 0)

#define FUNCSEL_TXRXDIS                 BIT(3)
/*------------------Baud Rate Compensation register---------------------------*/
#define BRCOMP_BRCOMP(value)            BIT_FIELD((value), 0)
#define BRCOMP_BRCOMP_MASK              BIT_FIELD(MASK(9), 0)
#define BRCOMP_BRCOMP_VALUE(reg) \
    FIELD_VALUE((reg), BRCOMP_BRCOMP_MASK, 0)

#define BRCOMP_BRCOMPDEC                BIT(31)
/*------------------Wake-up Control register----------------------------------*/
#define WKCTL_WKCTSEN                   BIT(0)
#define WKCTL_WKDATEN                   BIT(1)
#define WKCTL_WKRFRTEN                  BIT(2)
#define WKCTL_WKRS485EN                 BIT(3)
#define WKCTL_WKTOUTEN                  BIT(4)
/*------------------Wake-up Status register-----------------------------------*/
#define WKSTS_CTSWKF                    BIT(0)
#define WKSTS_DATWKF                    BIT(1)
#define WKSTS_RFRTWKF                   BIT(2)
#define WKSTS_RS485WKF                  BIT(3)
#define WKSTS_TOUTWKF                   BIT(4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_UART_DEFS_H_ */
