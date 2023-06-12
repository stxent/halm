/*
 * halm/platform/stm32/gen_2/uart_defs.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_2_UART_DEFS_H_
#define HALM_PLATFORM_STM32_GEN_2_UART_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT                   5
#define CHANNEL_INDEX(channel, index)   ((channel) * CHANNEL_COUNT + (index))
#define CHANNEL_RX(channel)             ((channel) * CHANNEL_COUNT + 0)
#define CHANNEL_TX(channel)             ((channel) * CHANNEL_COUNT + 1)
#define CHANNEL_CK(channel)             ((channel) * CHANNEL_COUNT + 2)
#define CHANNEL_RTS(channel)            ((channel) * CHANNEL_COUNT + 3)
#define CHANNEL_CTS(channel)            ((channel) * CHANNEL_COUNT + 4)
/*------------------Baud Rate Register----------------------------------------*/
#define BRR_DIV(value)                  BIT_FIELD((value), 0)
#define BRR_DIV_MASK                    BIT_FIELD(MASK(16), 0)
#define BRR_DIV_VALUE(reg)              FIELD_VALUE((reg), BRR_DIV_MASK, 0)
#define BRR_DIV_MAX                     65535
/*------------------Control Register 1----------------------------------------*/
/* USART enable */
#define CR1_UE                          BIT(0)
/* USART enable in stop mode */
#define CR1_UESM                        BIT(1)
/* Receiver enable */
#define CR1_RE                          BIT(2)
/* Transmitter enable */
#define CR1_TE                          BIT(3)
/* IDLE interrupt enable */
#define CR1_IDLEIE                      BIT(4)
/* RXNE interrupt enable */
#define CR1_RXNEIE                      BIT(5)
/* Transmission complete interrupt enable */
#define CR1_TCIE                        BIT(6)
/* TXE interrupt enable */
#define CR1_TXEIE                       BIT(7)
/* PE interrupt enable */
#define CR1_PEIE                        BIT(8)
/* Parity selection */
#define CR1_PS                          BIT(9)
/* Parity control enable */
#define CR1_PCE                         BIT(10)
/* Receiver wakeup method */
#define CR1_WAKE                        BIT(11)
/* Word length */
#define CR1_M0                          BIT(12)
/* Mute mode enable */
#define CR1_MME                         BIT(13)
/* Character match interrupt enable */
#define CR1_CMIE                        BIT(14)
/* Oversampling mode */
#define CR1_OVER8                       BIT(15)

/* Driver Enable de-assertion time */
#define CR1_DEDT(value)                 BIT_FIELD((value), 16)
#define CR1_DEDT_MASK                   BIT_FIELD(MASK(5), 16)
#define CR1_DEDT_VALUE(reg)             FIELD_VALUE((reg), CR1_DEDT_MASK, 16)

/* Driver Enable assertion time */
#define CR1_DEAT(value)                 BIT_FIELD((value), 21)
#define CR1_DEAT_MASK                   BIT_FIELD(MASK(5), 21)
#define CR1_DEAT_VALUE(reg)             FIELD_VALUE((reg), CR1_DEAT_MASK, 21)

/* Receiver timeout interrupt enable */
#define CR1_RTOIE                       BIT(26)
/* End of Block interrupt enable */
#define CR1_EOBIE                       BIT(27)
/* Word length */
#define CR1_M1                          BIT(28)
/*------------------Control Register 2----------------------------------------*/
/* 7-bit Address Detection or 4-bit Address Detection */
#define CR2_ADDM7                       BIT(4)
/* LIN break detection length */
#define CR2_LBDL                        BIT(4)
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
/* Swap TX and RX pins */
#define CR2_SWAP                        BIT(15)
/* RX pin active level inversion */
#define CR2_RXINV                       BIT(16)
/* TX pin active level inversion */
#define CR2_TXINV                       BIT(17)
/* Binary data inversion */
#define CR2_DATAINV                     BIT(18)
/* Most significant bit first */
#define CR2_MSBFIRST                    BIT(19)
/* Auto baud rate enable */
#define CR2_ABREN                       BIT(20)

/* Auto baud rate mode */
#define CR2_ABRMOD(value)               BIT_FIELD((value), 21)
#define CR2_ABRMOD_MASK                 BIT_FIELD(MASK(2), 21)
#define CR2_ABRMOD_VALUE(reg)           FIELD_VALUE((reg), CR2_ABRMOD_MASK, 21)

/* Receiver timeout enable */
#define CR2_RTOEN                       BIT(23)

/* Address of the USART node */
#define CR2_ADD(value)                  BIT_FIELD((value), 24)
#define CR2_ADD_MASK                    BIT_FIELD(MASK(8), 24)
#define CR2_ADD_VALUE(reg)              FIELD_VALUE((reg), CR2_ADD_MASK, 24)
/*------------------Control Register 3----------------------------------------*/
/* Error interrupt enable */
#define CR3_EIE                         BIT(0)
/* IrDA mode enable */
#define CR3_IREN                        BIT(1)
/* IrDA low-power */
#define CR3_IRLP                        BIT(2)
/* Half-duplex selection */
#define CR3_HDSEL                       BIT(3)
/* Smartcard NACK enable  */
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
/* One sample bit method enable */
#define CR3_ONEBIT                      BIT(11)
/* Overrun disable */
#define CR3_OVRDIS                      BIT(12)
/* DMA Disable on reception error */
#define CR3_DDRE                        BIT(13)
/* Driver enable mode */
#define CR3_DEM                         BIT(14)
/* Driver enable polarity selection */
#define CR3_DEP                         BIT(15)

/* Smartcard auto-retry count */
#define CR3_SCARCNT(value)              BIT_FIELD((value), 17)
#define CR3_SCARCNT_MASK                BIT_FIELD(MASK(3), 17)
#define CR3_SCARCNT_VALUE(reg)          FIELD_VALUE((reg), CR3_SCARCNT_MASK, 17)

/* Wakeup from stop mode interrupt flag selection */
#define CR3_WUS(value)                  BIT_FIELD((value), 20)
#define CR3_WUS_MASK                    BIT_FIELD(MASK(2), 20)
#define CR3_WUS_VALUE(reg)              FIELD_VALUE((reg), CR3_WUS_MASK, 20)

/* Wakeup from stop mode interrupt enable */
#define CR3_WUFIE                       BIT(22)
/*------------------Guard Time and Prescaler Register-------------------------*/
/* Prescaler value */
#define GTPR_PSC(value)                 BIT_FIELD((value), 0)
#define GTPR_PSC_MASK                   BIT_FIELD(MASK(8), 0)
#define GTPR_PSC_VALUE(reg)             FIELD_VALUE((reg), GTPR_PSC_MASK, 0)

/* Guard time value */
#define GTPR_GT(value)                  BIT_FIELD((value), 8)
#define GTPR_GT_MASK                    BIT_FIELD(MASK(8), 8)
#define GTPR_GT_VALUE(reg)              FIELD_VALUE((reg), GTPR_GT_MASK, 8)
/*------------------Receiver Timeout Register---------------------------------*/
/* Receiver timeout value */
#define RTOR_RTO(value)                 BIT_FIELD((value), 0)
#define RTOR_RTO_MASK                   BIT_FIELD(MASK(24), 0)
#define RTOR_RTO_VALUE(reg)             FIELD_VALUE((reg), RTOR_RTO_MASK, 0)

/* Block length */
#define RTOR_BLEN(value)                BIT_FIELD((value), 24)
#define RTOR_BLEN_MASK                  BIT_FIELD(MASK(8), 24)
#define RTOR_BLEN_VALUE(reg)            FIELD_VALUE((reg), RTOR_BLEN_MASK, 24)
/*------------------Request Register------------------------------------------*/
/* Auto baud rate request */
#define RQR_ABRRQ                       BIT(0)
/* Send break request */
#define RQR_SBKRQ                       BIT(1)
/* Mute mode request */
#define RQR_MMRQ                        BIT(2)
/* Receive data flush request */
#define RQR_RXFRQ                       BIT(3)
/* Transmit data flush request */
#define RQR_TXFRQ                       BIT(4)
/*------------------Interrupt and Status Register-----------------------------*/
/* Parity error */
#define ISR_PE                          BIT(0)
/* Framing error */
#define ISR_FE                          BIT(1)
/* Noise detection flag */
#define ISR_NF                          BIT(2)
/* Overrun error */
#define ISR_ORE                         BIT(3)
/* Idle line detected */
#define ISR_IDLE                        BIT(4)
/* Read data register not empty */
#define ISR_RXNE                        BIT(5)
/* Transmission complete */
#define ISR_TC                          BIT(6)
/* Transmit data register empty */
#define ISR_TXE                         BIT(7)
/* LIN break detection flag */
#define ISR_LBDF                        BIT(8)
/* CTS interrupt flag */
#define ISR_CTSIF                       BIT(9)
/* CTS flag */
#define ISR_CTS                         BIT(10)
/* Receiver timeout */
#define ISR_RTOF                        BIT(11)
/* End of block flag */
#define ISR_EOBF                        BIT(12)
/* Auto baud rate error */
#define ISR_ABRE                        BIT(14)
/* Auto baud rate flag */
#define ISR_ABRF                        BIT(15)
/* Busy flag */
#define ISR_BUSY                        BIT(16)
/* Character match flag */
#define ISR_CMF                         BIT(17)
/* Send break flag */
#define ISR_SBKF                        BIT(18)
/* Receiver wakeup from mute mode */
#define ISR_RWU                         BIT(19)
/* Wakeup from stop mode flag */
#define ISR_WUF                         BIT(20)
/* Transmit enable acknowledge flag */
#define ISR_TEACK                       BIT(21)
/* Receive enable acknowledge flag */
#define ISR_REACK                       BIT(22)
/*------------------Interrupt flag Clear Register-----------------------------*/
/* Parity error clear flag */
#define ICR_PECF                        BIT(0)
/* Framing error clear flag */
#define ICR_FECF                        BIT(1)
/* Noise detection clear flag */
#define ICR_NCF                         BIT(2)
/* Overrun error clear flag */
#define ICR_ORECF                       BIT(3)
/* Idle line detected clear flag */
#define ICR_IDLECF                      BIT(4)
/* Transmission complete clear flag */
#define ICR_TCCF                        BIT(6)
/* LIN break detection clear flag */
#define ICR_LBDCF                       BIT(8)
/* CTS interrupt clear flag */
#define ICR_CTSCF                       BIT(9)
/* Receiver timeout clear flag */
#define ICR_RTOCF                       BIT(11)
/* End of block clear flag */
#define ICR_EOBCF                       BIT(12)
/* Character match clear flag */
#define ICR_CMCF                        BIT(17)
/* Wakeup from stop mode clear flag */
#define ICR_WUCF                        BIT(20)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_UART_DEFS_H_ */
