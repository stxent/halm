/*
 * uart_defs.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_DEFS_H_
#define UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include "lpc17xx_defs.h"
/*----------------------------------------------------------------------------*/
/* UART register bits */
/* IrDA, Modem signals, auto-baud and RS-485 not used */
/*------------------Line Control Register-------------------------------------*/
#define LCR_WORD_5BIT                   BIT_FIELD(0, 0)
#define LCR_WORD_6BIT                   BIT_FIELD(1, 0)
#define LCR_WORD_7BIT                   BIT_FIELD(2, 0)
#define LCR_WORD_8BIT                   BIT_FIELD(3, 0)
#define LCR_STOP_1BIT                   BIT_FIELD(0, 2)
#define LCR_STOP_2BIT                   BIT_FIELD(1, 2)
#define LCR_PARITY                      BIT(3)
#define LCR_PARITY_ODD                  BIT_FIELD(0, 4)
#define LCR_PARITY_EVEN                 BIT_FIELD(1, 4)
#define LCR_BREAK                       BIT(6)
#define LCR_DLAB                        BIT(7)
/*------------------Interrupt Enable Register---------------------------------*/
#define IER_RBR                         BIT(0)
#define IER_THRE                        BIT(1)
#define IER_RX_LINE                     BIT(2)
/*------------------Interrupt Identification Register-------------------------*/
#define IIR_INT_STATUS                  BIT(0) /* Status, active low */
/* Mask for interrupt identification value */
#define IIR_INT_MASK                    BIT_FIELD(0x07, 1)
/* Receive Line Status */
#define IIR_INT_RLS                     BIT_FIELD(3, 1)
/* Receive Data Available */
#define IIR_INT_RDA                     BIT_FIELD(2, 1)
/* Character Timeout Interrupt */
#define IIR_INT_CTI                     BIT_FIELD(6, 1)
/* Transmitter Holding Register Empty interrupt */
#define IIR_INT_THRE                    BIT_FIELD(1, 1)
/*------------------FIFO Control Register-------------------------------------*/
#define FCR_ENABLE                      BIT(0)
#define FCR_RX_RESET                    BIT(1)
#define FCR_TX_RESET                    BIT(2)
/*
 * Level 0:  1 character
 * Level 1:  4 characters
 * Level 2:  8 characters
 * Level 3: 14 characters
 */
#define FCR_RX_TRIGGER(level)           BIT_FIELD((level), 6)
/*------------------Line Status Register--------------------------------------*/
#define LSR_RDR                         BIT(0) /* Receiver data ready */
#define LSR_OE                          BIT(1) /* Overrun error */
#define LSR_PE                          BIT(2) /* Parity error */
#define LSR_FE                          BIT(3) /* Framing error */
#define LSR_BI                          BIT(4) /* Break interrupt */
/* Transmitter holding register empty */
#define LSR_THRE                        BIT(5)
#define LSR_TEMT                        BIT(6) /* Transmitter empty */
#define LSR_RXFE                        BIT(7) /* Error in RX FIFO */
/*------------------Transmit Enable Register----------------------------------*/
#define TER_TXEN                        BIT(7) /* Transmit enable */
/*----------------------------------------------------------------------------*/
/* UART settings */
#define TX_FIFO_SIZE                    8
#define RX_FIFO_LEVEL                   2 /* 8 characters */
#define FRACTION_VALUE                  15 /* Divisor from 1 to 15 */
#define DEFAULT_DIV                     PCLK_DIV1
#define DEFAULT_DIV_VALUE               1
/*----------------------------------------------------------------------------*/
#endif /* UART_DEFS_H_ */
