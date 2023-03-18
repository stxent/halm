/*
 * halm/platform/lpc/gen_1/i2c_defs.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_1_I2C_DEFS_H_
#define HALM_PLATFORM_LPC_GEN_1_I2C_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*----------------------------------------------------------------------------*/
enum
{
  /* Master transmitter and receiver modes */

  STATUS_BUS_ERROR                    = 0x00,
  /* Start condition transmitted */
  STATUS_START_TRANSMITTED            = 0x08,
  /* Repeated start condition transmitted */
  STATUS_RESTART_TRANSMITTED          = 0x10,
  STATUS_SLAVE_WRITE_ACK              = 0x18,
  STATUS_SLAVE_WRITE_NACK             = 0x20,
  STATUS_MASTER_DATA_TRANSMITTED_ACK  = 0x28,
  STATUS_MASTER_DATA_TRANSMITTED_NACK = 0x30,
  STATUS_ARBITRATION_LOST             = 0x38,
  STATUS_SLAVE_READ_ACK               = 0x40,
  STATUS_SLAVE_READ_NACK              = 0x48,
  STATUS_MASTER_DATA_RECEIVED_ACK     = 0x50,
  STATUS_MASTER_DATA_RECEIVED_NACK    = 0x58,
  STATUS_NO_INFORMATION               = 0xF8,

  /* Slave receiver and transmitter modes except general call modes */

  STATUS_ADDRESS_WRITE_RECEIVED       = 0x60,
  STATUS_SLAVE_DATA_RECEIVED_ACK      = 0x80,
  STATUS_SLAVE_DATA_RECEIVED_NACK     = 0x88,
  STATUS_STOP_RECEIVED                = 0xA0,
  STATUS_ADDRESS_READ_RECEIVED        = 0xA8,
  STATUS_SLAVE_DATA_TRANSMITTED_ACK   = 0xB8,
  STATUS_SLAVE_DATA_TRANSMITTED_NACK  = 0xC0,
  STATUS_LAST_TRANSMITTED_ACK         = 0xC8
};
/*------------------Control Set register--------------------------------------*/
#define CONSET_AA                       BIT(2) /* Assert Acknowledge flag */
#define CONSET_SI                       BIT(3) /* I2C interrupt flag */
#define CONSET_STO                      BIT(4) /* STOP flag */
#define CONSET_STA                      BIT(5) /* START flag */
#define CONSET_I2EN                     BIT(6) /* I2C interface enable */
/*------------------Control Clear register------------------------------------*/
/* Writing a 1 clears corresponding bit, writing 0 has no effect */
#define CONCLR_AAC                      BIT(2)
#define CONCLR_SIC                      BIT(3)
#define CONCLR_STAC                     BIT(5)
#define CONCLR_I2ENC                    BIT(6)
/*------------------Data register---------------------------------------------*/
#define DATA_READ                       BIT(0)
#define DATA_WRITE                      0
/*------------------Monitor Mode Control register-----------------------------*/
#define MMCTRL_MM_ENA                   BIT(0) /* Monitor mode enable */
#define MMCTRL_ENA_SCL                  BIT(1) /* SCL output enable */
#define MMCTRL_MATCH_ALL                BIT(2) /* Select interrupt match mode */
/*------------------Slave Address registers-----------------------------------*/
#define ADR_GENERAL_CALL                BIT(0) /* General Call enable bit */
#define ADR_ADDRESS(value)              BIT_FIELD((value), 1)
#define ADR_ADDRESS_MASK                BIT_FIELD(MASK(7), 1)
#define ADR_ADDRESS_VALUE(reg)          FIELD_VALUE((reg), ADR_ADDRESS_MASK, 1)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_I2C_DEFS_H_ */
