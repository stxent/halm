/*
 * platform/nxp/i2c.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef I2C_H_
#define I2C_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include "i2c_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *I2c;
/*----------------------------------------------------------------------------*/
enum i2cState
{
  I2C_IDLE = 0,
  I2C_ADDRESS,
  I2C_TRANSMIT,
  I2C_RECEIVE,
  I2C_ERROR
};
/*----------------------------------------------------------------------------*/
enum i2cOption
{
  /**
   * Send stop after write operation. Option controls the repeated start
   * condition generation. This option is enabled by default.
   */
  IF_I2C_SENDSTOP = IF_END_OPTION
};
/*----------------------------------------------------------------------------*/
struct I2cConfig
{
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: serial clock pin. */
  gpio_t scl;
  /** Mandatory: serial data pin. */
  gpio_t sda;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct I2c
{
  struct I2cBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Bytes left for transmission or reception */
  uint16_t rxLeft, txLeft;

  /* Current interface state */
  enum i2cState state;
  /* Address of the device, only 7-bit addressing is supported */
  uint8_t address;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Generate stop condition after writing */
  bool sendStopBit;
};
/*----------------------------------------------------------------------------*/
#endif /* I2C_H_ */
