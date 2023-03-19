/*
 * halm/platform/numicro/i2c_defs.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_I2C_DEFS_H_
#define HALM_PLATFORM_NUMICRO_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
enum
{
  STATUS_BUS_ERROR              = 0x00,
  /* Start condition transmitted */
  STATUS_START_TRANSMITTED      = 0x08,
  /* Repeated start condition transmitted */
  STATUS_RESTART_TRANSMITTED    = 0x10,
  STATUS_SLAVE_WRITE_ACK        = 0x18,
  STATUS_SLAVE_WRITE_NACK       = 0x20,
  STATUS_DATA_TRANSMITTED_ACK   = 0x28,
  STATUS_DATA_TRANSMITTED_NACK  = 0x30,
  STATUS_ARBITRATION_LOST       = 0x38,
  STATUS_SLAVE_READ_ACK         = 0x40,
  STATUS_SLAVE_READ_NACK        = 0x48,
  STATUS_DATA_RECEIVED_ACK      = 0x50,
  STATUS_DATA_RECEIVED_NACK     = 0x58,
  STATUS_BUS_RELEASED           = 0xF8
};
/*------------------Control Register 0----------------------------------------*/
#define CTL0_AA                         BIT(2) /* Assert Acknowledge flag */
#define CTL0_SI                         BIT(3) /* I2C interrupt flag */
#define CTL0_STO                        BIT(4) /* STOP control */
#define CTL0_STA                        BIT(5) /* START control */
#define CTL0_I2CEN                      BIT(6) /* I2C controller enable */
#define CTL0_INTEN                      BIT(7) /* I2C interrupt enable */
/*------------------Data register---------------------------------------------*/
#define DATA_READ                       BIT(0)
#define DATA_RW_MASK                    BIT(0)
#define DATA_WRITE                      0
/*------------------Clock Divider register------------------------------------*/
#define CLKDIV_MAX                      MASK(10)
/*------------------Time-out Control register---------------------------------*/
#define TOCTL_TOIF                      BIT(0)
#define TOCTL_TOCDIV4                   BIT(1)
#define TOCTL_TOCEN                     BIT(2)
/*------------------Slave Address register------------------------------------*/
#define ADDR_GENERAL_CALL               BIT(0) /* General Call enable bit */
#define ADDR_ADDRESS(value)             BIT_FIELD((value), 1)
#define ADDR_ADDRESS_MASK               BIT_FIELD(MASK(10), 1)
#define ADDR_ADDRESS_VALUE(reg)         FIELD_VALUE((reg), ADDR_ADDRESS_MASK, 1)
/*------------------Slave Address Mask register-------------------------------*/
#define ADDRMSK_ADDRESS(value)          BIT_FIELD((value), 1)
#define ADDRMSK_ADDRESS_MASK            BIT_FIELD(MASK(10), 1)
#define ADDRMSK_ADDRESS_VALUE(reg) \
    FIELD_VALUE((reg), ADDRMSK_ADDRESS_MASK, 1)
/*------------------Wake-up Control register----------------------------------*/
#define WKCTL_WKEN                      BIT(0)
#define WKCTL_NHDBUSEN                  BIT(7)
/*------------------Wake-up Status register-----------------------------------*/
#define WKSTS_WKIF                      BIT(0)
#define WKSTS_WKAKDONE                  BIT(1)
#define WKSTS_WRSTSWK                   BIT(2)
/*------------------Control Register 1----------------------------------------*/
#define CTL1_TXPDMAEN                   BIT(0)
#define CTL1_RXPDMAEN                   BIT(1)
#define CTL1_PDMARST                    BIT(2)
#define CTL1_PDMASTR                    BIT(8)
#define CTL1_ADDR10EN                   BIT(9)
/*------------------Status Register 1-----------------------------------------*/
#define STATUS1_ADMAT0                  BIT(0)
#define STATUS1_ADMAT1                  BIT(1)
#define STATUS1_ADMAT2                  BIT(2)
#define STATUS1_ADMAT3                  BIT(3)
#define STATUS1_ONBUSY                  BIT(8)
/*------------------Timing Configuration Control register---------------------*/
#define TMCTL_STCTL_MAX                 MASK(9)
#define TMCTL_STCTL_MASK                BIT_FIELD(MASK(9), 0)
#define TMCTL_STCTL(value)              BIT_FIELD((value), 0)
#define TMCTL_STCTL_VALUE(reg)          FIELD_VALUE((reg), TMCTL_STCTL_MASK, 0)

#define TMCTL_HTCTL_MAX                 MASK(9)
#define TMCTL_HTCTL_MASK                BIT_FIELD(MASK(9), 16)
#define TMCTL_HTCTL(value)              BIT_FIELD((value), 16)
#define TMCTL_HTCTL_VALUE(reg)          FIELD_VALUE((reg), TMCTL_HTCTL_MASK, 16)
/*------------------Bus Management Control register---------------------------*/
#define BUSCTL_ACKMEN                   BIT(0)
#define BUSCTL_PECEN                    BIT(1)
#define BUSCTL_BMDEN                    BIT(2)
#define BUSCTL_BMHEN                    BIT(3)
#define BUSCTL_ALERTEN                  BIT(4)
#define BUSCTL_SCTLOSTS                 BIT(5)
#define BUSCTL_SCTLOEN                  BIT(6)
#define BUSCTL_BUSEN                    BIT(7)
#define BUSCTL_PECTXEN                  BIT(8)
#define BUSCTL_TIDLE                    BIT(9)
#define BUSCTL_PECCLR                   BIT(10)
#define BUSCTL_ACKM9SI                  BIT(11)
#define BUSCTL_BCDIEN                   BIT(12)
#define BUSCTL_PECDIEN                  BIT(13)
/*------------------Bus Management Timer Control register---------------------*/
#define BUSTCTL_BUSTOEN                 BIT(0)
#define BUSTCTL_CLKTOEN                 BIT(1)
#define BUSTCTL_BUSTOIEN                BIT(2)
#define BUSTCTL_CLKTOIEN                BIT(3)
#define BUSTCTL_TORSTEN                 BIT(4)
/*------------------Bus Management Status register----------------------------*/
#define BUSSTS_BUSY                     BIT(0)
#define BUSSTS_BCDONE                   BIT(1)
#define BUSSTS_PECERR                   BIT(2)
#define BUSSTS_ALERT                    BIT(3)
#define BUSSTS_SCTLDIN                  BIT(4)
#define BUSSTS_BUSTO                    BIT(5)
#define BUSSTS_CLKTO                    BIT(6)
#define BUSSTS_PECDONE                  BIT(7)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_I2C_DEFS_H_ */
