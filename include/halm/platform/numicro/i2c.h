/*
 * halm/platform/numicro/i2c.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_I2C_H_
#define HALM_PLATFORM_NUMICRO_I2C_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/numicro/i2c_base.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2C;

struct I2CConfig
{
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: serial clock pin. */
  PinNumber scl;
  /** Mandatory: serial data pin. */
  PinNumber sda;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: enable 10-bit addressing mode. */
  bool extended;
};

struct I2C
{
  struct I2CBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Bytes to be received */
  uint16_t rxLeft;
  /* Bytes to be transmitted */
  uint16_t txLeft;

  /* Address of the device */
  uint16_t address;
  /* Current interface state */
  uint8_t state;
  /* Enable blocking mode */
  bool blocking;
  /* Enable 10-bit addressing mode */
  bool extended;
  /* Generate repeated start condition after a next transfer */
  bool sendRepeatedStart;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_I2C_H_ */
