/*
 * halm/platform/lpc/gen_2/spi_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPI_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_SPI_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_2_SPI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Configuration register------------------------------------*/
#define CFG_ENABLE                      BIT(0)
#define CFG_MASTER                      BIT(2)
#define CFG_LSBF                        BIT(3)
#define CFG_CPHA                        BIT(4)
#define CFG_CPOL                        BIT(5)
#define CFG_LOOP                        BIT(7)
#define CFG_SPOL(channel)               BIT(8 + (channel))
#define CFG_SPOL0                       BIT(8)
#define CFG_SPOL1                       BIT(9)
#define CFG_SPOL2                       BIT(10)
#define CFG_SPOL3                       BIT(11)
/*------------------Delay register--------------------------------------------*/
#define DLY_PRE_DELAY(value)            BIT_FIELD((value), 0)
#define DLY_PRE_DELAY_MASK              BIT_FIELD(MASK(4), 0)
#define DLY_PRE_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), DLY_PRE_DELAY_MASK, 0)

#define DLY_POST_DELAY(value)           BIT_FIELD((value), 4)
#define DLY_POST_DELAY_MASK             BIT_FIELD(MASK(4), 4)
#define DLY_POST_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), DLY_POST_DELAY_MASK, 4)

#define DLY_FRAME_DELAY(value)          BIT_FIELD((value), 8)
#define DLY_FRAME_DELAY_MASK            BIT_FIELD(MASK(4), 8)
#define DLY_FRAME_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), DLY_FRAME_DELAY_MASK, 8)

#define DLY_TRANSFER_DELAY(value)       BIT_FIELD((value), 12)
#define DLY_TRANSFER_DELAY_MASK         BIT_FIELD(MASK(4), 12)
#define DLY_TRANSFER_DELAY_VALUE(reg) \
    FIELD_VALUE((reg), DLY_TRANSFER_DELAY_MASK, 12)
/*------------------Status register-------------------------------------------*/
#define STAT_RXRDY                      BIT(0)
#define STAT_TXRDY                      BIT(1)
#define STAT_RXOV                       BIT(2)
#define STAT_TXUR                       BIT(3)
#define STAT_SSA                        BIT(4)
#define STAT_SSD                        BIT(5)
#define STAT_STALLED                    BIT(6)
#define STAT_ENDTRANSFER                BIT(7)
#define STAT_MSTIDLE                    BIT(8)
/*------------------Interrupt Enable read and Set register--------------------*/
#define INTENSET_RXRDYEN                BIT(0)
#define INTENSET_TXRDYEN                BIT(1)
#define INTENSET_RXOVEN                 BIT(2)
#define INTENSET_TXUREN                 BIT(3)
#define INTENSET_SSAEN                  BIT(4)
#define INTENSET_SSDEN                  BIT(5)
/*------------------Interrupt Enable Clear register---------------------------*/
#define INTENCLR_RXRDYEN                BIT(0)
#define INTENCLR_TXRDYEN                BIT(1)
#define INTENCLR_RXOVEN                 BIT(2)
#define INTENCLR_TXUREN                 BIT(3)
#define INTENCLR_SSAEN                  BIT(4)
#define INTENCLR_SSDEN                  BIT(5)
/*------------------Receiver Data register------------------------------------*/
#define RXDAT_RXDAT(value)              BIT_FIELD((value), 0)
#define RXDAT_RXDAT_MASK                BIT_FIELD(MASK(16), 0)
#define RXDAT_RXDAT_VALUE(reg)          FIELD_VALUE((reg), RXDAT_RXDAT_MASK, 0)

#define RXDAT_RXSSEL0_N                 BIT(16)
#define RXDAT_RXSSEL1_N                 BIT(17)
#define RXDAT_RXSSEL2_N                 BIT(18)
#define RXDAT_RXSSEL3_N                 BIT(19)
#define RXDAT_SOT                       BIT(20)
/*------------------Transmitter Data and Control register---------------------*/
#define TXDATCTL_TXDAT(value)           BIT_FIELD((value), 0)
#define TXDATCTL_TXDAT_MASK             BIT_FIELD(MASK(16), 0)
#define TXDATCTL_TXDAT_VALUE(reg) \
    FIELD_VALUE((reg), TXDATCTL_TXDAT_MASK, 0)

#define TXDATCTL_TXSSEL0_N              BIT(16)
#define TXDATCTL_TXSSEL1_N              BIT(17)
#define TXDATCTL_TXSSEL2_N              BIT(18)
#define TXDATCTL_TXSSEL3_N              BIT(19)
#define TXDATCTL_EOT                    BIT(20)
#define TXDATCTL_EOF                    BIT(21)
#define TXDATCTL_RXIGNORE               BIT(22)

#define TXDATCTL_LEN(value)             BIT_FIELD((value), 24)
#define TXDATCTL_LEN_MASK               BIT_FIELD(MASK(4), 24)
#define TXDATCTL_LEN_VALUE(reg) \
    FIELD_VALUE((reg), TXDATCTL_LEN_MASK, 24)
/*------------------Transmitter Control register------------------------------*/
#define TXCTL_TXSSEL0_N                 BIT(16)
#define TXCTL_TXSSEL1_N                 BIT(17)
#define TXCTL_TXSSEL2_N                 BIT(18)
#define TXCTL_TXSSEL3_N                 BIT(19)
#define TXCTL_EOT                       BIT(20)
#define TXCTL_EOF                       BIT(21)
#define TXCTL_RXIGNORE                  BIT(22)

#define TXCTL_LEN(value)                BIT_FIELD((value), 24)
#define TXCTL_LEN_MASK                  BIT_FIELD(MASK(4), 24)
#define TXCTL_LEN_VALUE(reg)            FIELD_VALUE((reg), TXCTL_LEN_MASK, 24)
/*------------------Divider register------------------------------------------*/
#define DIV_DIVVAL_MAX                  MASK(16)
/*------------------Interrupt Status register---------------------------------*/
#define INTSTAT_RXRDY                   BIT(0)
#define INTSTAT_TXRDY                   BIT(1)
#define INTSTAT_RXOV                    BIT(2)
#define INTSTAT_TXUR                    BIT(3)
#define INTSTAT_SSA                     BIT(4)
#define INTSTAT_SSD                     BIT(5)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_SPI_DEFS_H_ */
