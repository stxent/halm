/*
 * halm/platform/numicro/spi_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SPI_DEFS_H_
#define HALM_PLATFORM_NUMICRO_SPI_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------Control register------------------------------------------*/
#define CTL_SPIEN                       BIT(0)
#define CTL_RXNEG                       BIT(1)
#define CTL_TXNEG                       BIT(2)
#define CTL_CLKPOL                      BIT(3)

#define CTL_SUSPITV(value)              BIT_FIELD((value), 4)
#define CTL_SUSPITV_MASK                BIT_FIELD(MASK(4), 4)
#define CTL_SUSPITV_VALUE(reg)          FIELD_VALUE((reg), CTL_SUSPITV_MASK, 4)

#define CTL_DWIDTH(value)               BIT_FIELD((value), 8)
#define CTL_DWIDTH_MASK                 BIT_FIELD(MASK(4), 8)
#define CTL_DWIDTH_VALUE(reg)           FIELD_VALUE((reg), CTL_DWIDTH_MASK, 8)

#define CTL_LSB                         BIT(13)
#define CTL_HALFDPX                     BIT(14)
#define CTL_RXONLY                      BIT(15)
#define CTL_UNITIEN                     BIT(17)
#define CTL_SLAVE                       BIT(18)
#define CTL_REORDER                     BIT(19)
#define CTL_DATDIR                      BIT(20)
/*------------------Clock Divider register------------------------------------*/
#define CLKDIV_DIVIDER_MASK             BIT_FIELD(MASK(9), 0)
/*------------------Slave Select Control register-----------------------------*/
#define SSCTL_SS                        BIT(0)
#define SSCTL_SSACTPOL                  BIT(2)
#define SSCTL_AUTOSS                    BIT(3)
#define SSCTL_SLVBEIEN                  BIT(8)
#define SSCTL_SLVURIEN                  BIT(9)
#define SSCTL_SSACTIEN                  BIT(12)
#define SSCTL_SSINAIEN                  BIT(13)
/*------------------PDMA Control register-------------------------------------*/
#define PDMACTL_TXPDMAEN                BIT(0)
#define PDMACTL_RXPDMAEN                BIT(1)
#define PDMACTL_PDMARST                 BIT(2)
/*------------------FIFO Control register-------------------------------------*/
#define FIFOCTL_RXRST                   BIT(0)
#define FIFOCTL_TXRST                   BIT(1)
#define FIFOCTL_RXTHIEN                 BIT(2)
#define FIFOCTL_TXTHIEN                 BIT(3)
#define FIFOCTL_RXTOIEN                 BIT(4)
#define FIFOCTL_RXOVIEN                 BIT(5)
#define FIFOCTL_TXUFPOL                 BIT(6)
#define FIFOCTL_TXUFIEN                 BIT(7)
#define FIFOCTL_RXFBCLR                 BIT(8)
#define FIFOCTL_TXFBCLR                 BIT(9)

#define FIFOCTL_RXTH(value)             BIT_FIELD((value), 24)
#define FIFOCTL_RXTH_MASK               BIT_FIELD(MASK(3), 24)
#define FIFOCTL_RXTH_VALUE(reg) \
    FIELD_VALUE((reg), FIFOCTL_RXTH_MASK, 24)

#define FIFOCTL_TXTH(value)             BIT_FIELD((value), 28)
#define FIFOCTL_TXTH_MASK               BIT_FIELD(MASK(3), 28)
#define FIFOCTL_TXTH_VALUE(reg) \
    FIELD_VALUE((reg), FIFOCTL_TXTH_MASK, 28)
/*------------------Status register-------------------------------------------*/
#define STATUS_BUSY                     BIT(0)
#define STATUS_UNITIF                   BIT(1)
#define STATUS_SSACTIF                  BIT(2)
#define STATUS_SSINAIF                  BIT(3)
#define STATUS_SSLINE                   BIT(4)
#define STATUS_SLVBEIF                  BIT(6)
#define STATUS_SLVURIF                  BIT(7)
#define STATUS_RXEMPTY                  BIT(8)
#define STATUS_RXFULL                   BIT(9)
#define STATUS_RXTHIF                   BIT(10)
#define STATUS_RXOVIF                   BIT(11)
#define STATUS_RXTOIF                   BIT(12)
#define STATUS_SPIENSTS                 BIT(15)
#define STATUS_TXEMPTY                  BIT(16)
#define STATUS_TXFULL                   BIT(17)
#define STATUS_TXTHIF                   BIT(18)
#define STATUS_TXUFIF                   BIT(19)
#define STATUS_TXRXRST                  BIT(23)

#define STATUS_RXCNT(value)             BIT_FIELD((value), 24)
#define STATUS_RXCNT_MASK               BIT_FIELD(MASK(3), 24)
#define STATUS_RXCNT_VALUE(reg) \
    FIELD_VALUE((reg), STATUS_RXCNT_MASK, 24)

#define STATUS_TXCNT(value)             BIT_FIELD((value), 24)
#define STATUS_TXCNT_MASK               BIT_FIELD(MASK(3), 24)
#define STATUS_TXCNT_VALUE(reg) \
    FIELD_VALUE((reg), STATUS_TXCNT_MASK, 24)
/*------------------Status 2 register-----------------------------------------*/
#define STATUS2_SLVBENUM(value)         BIT_FIELD((value), 24)
#define STATUS2_SLVBENUM_MASK           BIT_FIELD(MASK(6), 24)
#define STATUS2_SLVBENUM_VALUE(reg) \
    FIELD_VALUE((reg), STATUS2_SLVBENUM_MASK, 24)
/*------------------I2S Control register--------------------------------------*/
enum
{
  FORMAT_I2S            = 0,
  FORMAT_MSB_JUSTIFIED  = 1,
  FORMAT_PCM_MODE_A     = 2,
  FORMAT_PCM_MODE_B     = 3
};

#define I2SCTL_I2SEN                    BIT(0)
#define I2SCTL_TXEN                     BIT(1)
#define I2SCTL_RXEN                     BIT(2)
#define I2SCTL_MUTE                     BIT(3)

#define I2SCTL_WDWIDTH(value)           BIT_FIELD((value), 4)
#define I2SCTL_WDWIDTH_MASK             BIT_FIELD(MASK(2), 4)
#define I2SCTL_WDWIDTH_VALUE(reg) \
    FIELD_VALUE((reg), I2SCTL_WDWIDTH_MASK, 4)

#define I2SCTL_MONO                     BIT(6)
#define I2SCTL_ORDER                    BIT(7)
#define I2SCTL_SLAVE                    BIT(8)
#define I2SCTL_MCLKEN                   BIT(15)
#define I2SCTL_RZCEN                    BIT(16)
#define I2SCTL_LZCEN                    BIT(17)
#define I2SCTL_RXLCH                    BIT(23)
#define I2SCTL_RZCIEN                   BIT(24)
#define I2SCTL_LZCIEN                   BIT(25)

#define I2SCTL_FORMAT(value)            BIT_FIELD((value), 28)
#define I2SCTL_FORMAT_MASK              BIT_FIELD(MASK(2), 28)
#define I2SCTL_FORMAT_VALUE(reg) \
    FIELD_VALUE((reg), I2SCTL_FORMAT_MASK, 28)

#define I2SCTL_SLVERRIEN                BIT(31)
/*------------------I2S Clock Divider Control register------------------------*/
#define I2SCLK_MCLKDIV(value)           BIT_FIELD((value), 0)
#define I2SCLK_MCLKDIV_MASK             BIT_FIELD(MASK(7), 0)
#define I2SCLK_MCLKDIV_VALUE(reg) \
    FIELD_VALUE((reg), I2SCLK_MCLKDIV_MASK, 0)

#define I2SCLK_BCLKDIV(value)           BIT_FIELD((value), 8)
#define I2SCLK_BCLKDIV_MASK             BIT_FIELD(MASK(10), 8)
#define I2SCLK_BCLKDIV_VALUE(reg) \
    FIELD_VALUE((reg), I2SCLK_BCLKDIV_MASK, 8)
/*------------------I2S Status register---------------------------------------*/
#define I2SSTS_RIGHT                    BIT(4)
#define I2SSTS_RXEMPTY                  BIT(8)
#define I2SSTS_RXFULL                   BIT(9)
#define I2SSTS_RXTHIF                   BIT(10)
#define I2SSTS_RXOVIF                   BIT(11)
#define I2SSTS_RXTOIF                   BIT(12)
#define I2SSTS_I2SENSTS                 BIT(15)
#define I2SSTS_TXEMPTY                  BIT(16)
#define I2SSTS_TXFULL                   BIT(17)
#define I2SSTS_TXTHIF                   BIT(18)
#define I2SSTS_TXUFIF                   BIT(19)
#define I2SSTS_RZCIF                    BIT(20)
#define I2SSTS_LZCIF                    BIT(21)
#define I2SSTS_SLVERRIF                 BIT(22)
#define I2SSTS_TXRXRST                  BIT(21)

#define I2SSTS_RXCNT(value)             BIT_FIELD((value), 24)
#define I2SSTS_RXCNT_MASK               BIT_FIELD(MASK(3), 24)
#define I2SSTS_RXCNT_VALUE(reg) \
    FIELD_VALUE((reg), I2SSTS_RXCNT_MASK, 24)

#define I2SSTS_TXCNT(value)             BIT_FIELD((value), 24)
#define I2SSTS_TXCNT_MASK               BIT_FIELD(MASK(3), 24)
#define I2SSTS_TXCNT_VALUE(reg) \
    FIELD_VALUE((reg), I2SSTS_TXCNT_MASK, 24)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SPI_DEFS_H_ */
