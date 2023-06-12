/*
 * halm/platform/stm32/gen_1/uart_defs.h
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_1_UART_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_1_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Status Register-------------------------------------------*/
/* Parity error flag */
#define SR_PE                           BIT(0)
/* Framing error flag */
#define SR_FE                           BIT(1)
/* Noise error flag */
#define SR_NE                           BIT(2)
/* Overrun error */
#define SR_ORE                          BIT(3)
/* IDLE line detected */
#define SR_IDLE                         BIT(4)
/* RDR not empty */
#define SR_RXNE                         BIT(5)
/* Transmission complete */
#define SR_TC                           BIT(6)
/* Transmit data register empty */
#define SR_TXE                          BIT(7)
/* LIN break detection flag */
#define SR_LBD                          BIT(8)
/* CTS flag */
#define SR_CTS                          BIT(9)
/*------------------Baud Rate Register----------------------------------------*/
#define BRR_DIV(value)                  BIT_FIELD((value), 0)
#define BRR_DIV_MASK                    BIT_FIELD(MASK(16), 0)
#define BRR_DIV_VALUE(reg)              FIELD_VALUE((reg), BRR_DIV_MASK, 0)
#define BRR_DIV_MAX                     65535
/*------------------Control Register 1----------------------------------------*/
/* Send break */
#define CR1_SBK                         BIT(0)
/* Receiver wakeup */
#define CR1_RWU                         BIT(1)
/* Receiver enable */
#define CR1_RE                          BIT(2)
/* Transmitter enable */
#define CR1_TE                          BIT(3)
/* IDLE interrupt enable */
#define CR1_IDLEIE                      BIT(4)
/* RXNE interrupt enable */
#define CR1_RXNEIE                      BIT(5)
/* TC interrupt enable */
#define CR1_TCIE                        BIT(6)
/* TXE interrupt enable */
#define CR1_TXEIE                       BIT(7)
/* PE interrupt enable */
#define CR1_PEIE                        BIT(8)
/* Parity selection */
#define CR1_PS                          BIT(9)
/* Parity control enable */
#define CR1_PCE                         BIT(10)
/* Wakeup method */
#define CR1_WAKE                        BIT(11)
/* Word length */
#define CR1_M                           BIT(12)
#define CR1_M0                          BIT(12)
/* USART enable */
#define CR1_UE                          BIT(13)
/*------------------Control Register 2----------------------------------------*/
enum
{
  STOP_1_BIT,
  STOP_0_5_BIT,
  STOP_2_BITS,
  STOP_1_5_BITS
};

/* Address of the USART node */
#define CR2_ADD(value)                  BIT_FIELD((value), 0)
#define CR2_ADD_MASK                    BIT_FIELD(MASK(4), 0)
#define CR2_ADD_VALUE(reg)              FIELD_VALUE((reg), CR2_ADD_MASK, 0)

/* LIN break detection length */
#define CR2_LBDL                        BIT(5)
/* LIN break detection interrupt enable */
#define CR2_LBDIE                       BIT(6)
/* Last bit clock pulse */
#define CR2_LBCL                        BIT(8)
/* Clock phase */
#define CR2_CPHA                        BIT(9)
/* Clock polarity */
#define CR2_CPOL                        BIT(10)
/* Clock enable */
#define CR2_CLKEN                       BIT(11)

/* Stop bits */
#define CR2_STOP(value)                 BIT_FIELD((value), 12)
#define CR2_STOP_MASK                   BIT_FIELD(MASK(2), 12)
#define CR2_STOP_VALUE(reg)             FIELD_VALUE((reg), CR2_STOP_MASK, 12)

/* LIN mode enable */
#define CR2_LINEN                       BIT(14)
/*------------------Control Register 3----------------------------------------*/
/* Error interrupt enable */
#define CR3_EIE                         BIT(0)
/* IrDA mode enable */
#define CR3_IREN                        BIT(1)
/* IrDA low power */
#define CR3_IRLP                        BIT(2)
/* Half-duplex selection */
#define CR3_HDSEL                       BIT(3)
/* Smartcard NACK enable */
#define CR3_NACK                        BIT(4)
/* Smartcard mode enable */
#define CR3_SCEN                        BIT(5)
/* DMA enable receiver */
#define CR3_DMAR                        BIT(6)
/* DMA enable transmitter */
#define CR3_DMAT                        BIT(7)
/* RTS enable */
#define CR3_RTSE                        BIT(8)
/* CTS enable */
#define CR3_CTSE                        BIT(9)
/* CTS interrupt enable */
#define CR3_CTSIE                       BIT(10)
/*------------------Guard Time and Prescaler Register-------------------------*/
/* Prescaler value */
#define GTPR_PSC(value)                 BIT_FIELD((value), 0)
#define GTPR_PSC_MASK                   BIT_FIELD(MASK(8), 0)
#define GTPR_PSC_VALUE(reg)             FIELD_VALUE((reg), GTPR_PSC_MASK, 0)

/* Guard time value */
#define GTPR_GT(value)                  BIT_FIELD((value), 8)
#define GTPR_GT_MASK                    BIT_FIELD(MASK(8), 8)
#define GTPR_GT_VALUE(reg)              FIELD_VALUE((reg), GTPR_GT_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_UART_DEFS_H_ */
