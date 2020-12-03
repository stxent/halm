/*
 * halm/platform/lpc/gen_1/uart_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_UART_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_1_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Line Control Register-------------------------------------*/
enum
{
  WLS_5BIT = 0,
  WLS_6BIT = 1,
  WLS_7BIT = 2,
  WLS_8BIT = 3
};

enum
{
  PS_ODD         = 0,
  PS_EVEN        = 1,
  PS_FORCED_HIGH = 2,
  PS_FORCED_LOW  = 3
};

#define LCR_WLS_MASK                    BIT_FIELD(MASK(2), 0)
#define LCR_WLS(length)                 BIT_FIELD((length), 0)
#define LCR_WLS_VALUE(reg)              FIELD_VALUE((reg), LCR_WLS_MASK, 0)
#define LCR_SBS_2BIT                    BIT(2)
#define LCR_PE                          BIT(3)
#define LCR_PS_MASK                     BIT_FIELD(MASK(2), 4)
#define LCR_PS(parity)                  BIT_FIELD((parity), 4)
#define LCR_PS_VALUE(reg)               FIELD_VALUE((reg), LCR_PS_MASK, 4)
#define LCR_BC                          BIT(6)
#define LCR_DLAB                        BIT(7)
/*------------------Interrupt Enable Register---------------------------------*/
#define IER_RBRINTEN                    BIT(0)
#define IER_THREINTEN                   BIT(1)
#define IER_RLSINTEN                    BIT(2)
#define IER_MSINTEN                     BIT(3)
#define IER_ABEOINTEN                   BIT(8)
#define IER_ABTOINTEN                   BIT(9)
/*------------------Interrupt Identification Register-------------------------*/
/* Status, active low */
#define IIR_INTSTATUS                   BIT(0)
/* Mask for interrupt identification value */
#define IIR_INTID_MASK                  BIT_FIELD(MASK(3), 1)
/* Receive Line Status */
#define IIR_INTID_RLS                   BIT_FIELD(3, 1)
/* Receive Data Available */
#define IIR_INTID_RDA                   BIT_FIELD(2, 1)
/* Character Timeout Interrupt */
#define IIR_INTID_CTI                   BIT_FIELD(6, 1)
/* Transmitter Holding Register Empty interrupt */
#define IIR_INTID_THRE                  BIT_FIELD(1, 1)
/* Modem Status interrupt */
#define IIR_INTID_MS                    BIT_FIELD(0, 1)
/* End of auto-baud interrupt */
#define IIR_ABEOINT                     BIT(8)
/* Auto-baud timeout interrupt */
#define IIR_ABTOINT                     BIT(9)
/*------------------FIFO Control Register-------------------------------------*/
/* How many characters must be received before interrupt */
enum
{
  RX_TRIGGER_LEVEL_1  = 0,
  RX_TRIGGER_LEVEL_4  = 1,
  RX_TRIGGER_LEVEL_8  = 2,
  RX_TRIGGER_LEVEL_14 = 3
};

#define FCR_FIFOEN                      BIT(0)
#define FCR_RXFIFORES                   BIT(1)
#define FCR_TXFIFORES                   BIT(2)
#define FCR_DMAMODE                     BIT(3)
#define FCR_RXTRIGLVL_MASK              BIT_FIELD(MASK(2), 6)
#define FCR_RXTRIGLVL(level)            BIT_FIELD((level), 6)
#define FCR_RXTRIGLVL_VALUE(reg) \
    FIELD_VALUE((reg), FCR_RXTRIGLVL_MASK, 6)
/*------------------Line Status Register--------------------------------------*/
/* Receiver data ready */
#define LSR_RDR                         BIT(0)
/* Overrun error */
#define LSR_OE                          BIT(1)
/* Parity error */
#define LSR_PE                          BIT(2)
/* Framing error */
#define LSR_FE                          BIT(3)
/* Break interrupt */
#define LSR_BI                          BIT(4)
/* Transmitter holding register empty */
#define LSR_THRE                        BIT(5)
/* Transmitter empty */
#define LSR_TEMT                        BIT(6)
/* Error in RX FIFO */
#define LSR_RXFE                        BIT(7)
/* Error in transmitted character */
#define LSR_TXERR                       BIT(8)
/*------------------Modem Status Register-------------------------------------*/
#define MSR_DCTS                        BIT(0)
#define MSR_DDSR                        BIT(1)
#define MSR_TERI                        BIT(2)
#define MSR_DDCD                        BIT(3)
#define MSR_CTS                         BIT(4)
#define MSR_DSR                         BIT(5)
#define MSR_RI                          BIT(6)
#define MSR_DCD                         BIT(7)
/*------------------Transmit Enable Register----------------------------------*/
/* Transmit enable */
#define TER_TXEN                        BIT(7)
/*------------------IrDA Control Register-------------------------------------*/
#define ICR_IRDAEN                      BIT(0)
#define ICR_IRDAINV                     BIT(1)
#define ICR_FIXPULSEEN                  BIT(2)
#define ICR_PULSEDIV_MASK               BIT_FIELD(MASK(3), 3)
#define ICR_PULSEDIV(level)             BIT_FIELD((level), 3)
#define ICR_PULSEDIV_VALUE(reg)         FIELD_VALUE((reg), ICR_PULSEDIV_MASK, 3)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_UART_DEFS_H_ */
