/*
 * halm/platform/lpc/ssp_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SSP_DEFS_H_
#define HALM_PLATFORM_LPC_SSP_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control Register 0----------------------------------------*/
enum
{
  FRF_SPI       = 0,
  FRF_TI        = 1,
  FRF_MICROWIRE = 2
};

/* Data Size Select */
#define CR0_DSS_MASK                    BIT_FIELD(MASK(4), 0)
/* Possible values are from 4 to 16 bit */
#define CR0_DSS(size)                   BIT_FIELD((size) - 1, 0)
/* Frame Format */
#define CR0_FRF_MASK                    BIT_FIELD(MASK(2), 4)
#define CR0_FRF(format)                 BIT_FIELD((format), 4)
#define CR0_FRF_VALUE(reg)              FIELD_VALUE((reg), CR0_FRF_MASK, 4)
/* Clock Out Polarity, only for SPI format */
/* When set, controller maintains clock high between frames */
#define CR0_CPOL                        BIT(6)
/* Clock Out Phase, only for SPI format */
/* When set, controller captures data on the second clock of the frame */
#define CR0_CPHA                        BIT(7)
/* Serial Clock Rate */
#define CR0_SCR_MASK                    BIT_FIELD(MASK(8), 8)
/* Resulting bit frequency is PCLK / (CPSDVSR * (SCR + 1)) */
#define CR0_SCR(rate)                   BIT_FIELD((rate), 8)
#define CR0_SCR_VALUE(reg)              FIELD_VALUE((reg), CR0_SCR_MASK, 8)
#define CR0_SCR_MAX                     255
/*------------------Control Register 1----------------------------------------*/
/* Loop-back mode */
#define CR1_LBM                         BIT(0)
/* SSP Enable */
#define CR1_SSE                         BIT(1)
/* Master-Slave mode, when set controller acts as slave */
#define CR1_MS                          BIT(2)
/* Slave Output Disable bit, is relevant only in slave mode */
#define CR1_SOD                         BIT(3)
/*------------------Status Register-------------------------------------------*/
#define SR_TFE                          BIT(0) /* Transmit FIFO empty */
#define SR_TNF                          BIT(1) /* Transmit FIFO not full */
#define SR_RNE                          BIT(2) /* Receive FIFO not empty */
#define SR_RFF                          BIT(3) /* Receive FIFO full */
#define SR_BSY                          BIT(4) /* Is the controller busy */
/*------------------Clock Prescaler Register----------------------------------*/
#define CPSR_MIN                        2
#define CPSR_MAX                        254
/*------------------Interrupt Mask Set/Clear register-------------------------*/
/* Enables or disables specific interrupts */
#define IMSC_RORIM                      BIT(0) /* Receive overrun */
#define IMSC_RTIM                       BIT(1) /* Receive timeout condition */
#define IMSC_RXIM                       BIT(2) /* Receive FIFO is half-full */
#define IMSC_TXIM                       BIT(3) /* Transmit FIFO is half-empty */
/*------------------Raw Interrupt Status register-----------------------------*/
#define RIS_RORRIS                      BIT(0)
#define RIS_RTRIS                       BIT(1)
#define RIS_RXRIS                       BIT(2)
#define RIS_TXRIS                       BIT(3)
/*------------------Masked Interrupt Status register--------------------------*/
#define MIS_RORMIS                      BIT(0)
#define MIS_RTMIS                       BIT(1)
#define MIS_RXMIS                       BIT(2)
#define MIS_TXMIS                       BIT(3)
/*------------------Interrupt Clear Register----------------------------------*/
#define ICR_RORIC                       BIT(0)
#define ICR_RTIC                        BIT(1)
/*------------------DMA Control Register--------------------------------------*/
/* Available only on devices with direct memory access support */
#define DMACR_RXDMAE                    BIT(0) /* Receive DMA enable */
#define DMACR_TXDMAE                    BIT(1) /* Transmit DMA enable */
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SSP_DEFS_H_ */
