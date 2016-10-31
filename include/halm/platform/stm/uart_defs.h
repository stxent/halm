/*
 * halm/platform/stm/stm32f1xx/uart_defs.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_UART_DEFS_H_
#define HALM_PLATFORM_STM_STM32F1XX_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Status Register-------------------------------------------*/
#define SR_PE                           BIT(0) /* Parity error flag */
#define SR_FE                           BIT(1) /* Framing error flag */
#define SR_NE                           BIT(2) /* Noise error flag */
#define SR_ORE                          BIT(3) /* Overrun error */
#define SR_IDLE                         BIT(4) /* IDLE line detected */
#define SR_RXNE                         BIT(5) /* RDR not empty */
#define SR_TC                           BIT(6) /* Transmission complete */
#define SR_TXE                          BIT(7) /* TDR empty */
#define SR_LBD                          BIT(8) /* LIN break detection flag */
#define SR_CTS                          BIT(9) /* CTS flag */
/*------------------Control Register 1----------------------------------------*/
#define CR1_SBK                         BIT(0) /* Send break */
#define CR1_RWU                         BIT(1) /* Receiver wakeup */
#define CR1_RE                          BIT(2) /* Receiver enable */
#define CR1_TE                          BIT(3) /* Transmitter enable */
#define CR1_IDLEIE                      BIT(4) /* IDLE interrupt enable */
#define CR1_RXNEIE                      BIT(5) /* RXNE interrupt enable */
#define CR1_TCIE                        BIT(6) /* TC interrupt enable */
#define CR1_TXEIE                       BIT(7) /* TXE interrupt enable */
#define CR1_PEIE                        BIT(8) /* PE interrupt enable */
#define CR1_PS                          BIT(9) /* Parity selection */
#define CR1_PCE                         BIT(10) /* Parity control enable */
#define CR1_WAKE                        BIT(11) /* Wakeup method */
#define CR1_M                           BIT(12) /* Word length */
#define CR1_UE                          BIT(13) /* USART enable */
/*------------------Control Register 2----------------------------------------*/
enum
{
  STOP_1_BIT,
  STOP_0_5_BIT,
  STOP_2_BITS,
  STOP_1_5_BITS
};

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

#define CR2_STOP(value)                 BIT_FIELD((value), 12)
#define CR2_STOP_MASK                   BIT_FIELD(MASK(2), 12)
#define CR2_STOP_VALUE(reg)             FIELD_VALUE((reg), CR2_STOP_MASK, 12)

/* LIN mode enable */
#define CR2_LINEN                       BIT(14)
/*------------------Control Register 3----------------------------------------*/
#define CR3_EIE                         BIT(0) /* Error interrupt enable */
#define CR3_IREN                        BIT(1) /* IrDA mode enable */
#define CR3_IRLP                        BIT(2) /* IrDA low power */
#define CR3_HDSEL                       BIT(3) /* Half-duplex selection */
#define CR3_NACK                        BIT(4) /* Smartcard NACK enable */
#define CR3_SCEN                        BIT(5) /* Smartcard mode enable */
#define CR3_DMAR                        BIT(6) /* DMA enable receiver */
#define CR3_DMAT                        BIT(7) /* DMA enable transmitter */
#define CR3_RTSE                        BIT(8) /* RTS enable */
#define CR3_CTSE                        BIT(9) /* CTS enable */
#define CR3_CTSIE                       BIT(10) /* CTS interrupt enable */
/*------------------Guard Time and Prescaler Register-------------------------*/
#define GTPR_PSC(value)                 BIT_FIELD((value), 0)
#define GTPR_PSC_MASK                   BIT_FIELD(MASK(8), 0)
#define GTPR_PSC_VALUE(reg)             FIELD_VALUE((reg), GTPR_PSC_MASK, 0)

#define GTPR_GT(value)                  BIT_FIELD((value), 8)
#define GTPR_GT_MASK                    BIT_FIELD(MASK(8), 8)
#define GTPR_GT_VALUE(reg)              FIELD_VALUE((reg), GTPR_GT_MASK, 8)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_UART_DEFS_H_ */
