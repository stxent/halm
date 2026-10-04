/*
 * halm/platform/bouffalo/i2c.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_I2C_H_
#define HALM_PLATFORM_BOUFFALO_I2C_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/bouffalo/i2c_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2C;

struct I2CConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Mandatory: serial data line pin. */
  PinNumber sda;
  /** Mandatory: serial clock line pin. */
  PinNumber scl;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct I2C
{
  struct I2CBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Slave address */
  uint32_t address;
  /* Desired data rate */
  uint32_t rate;
  /* Pointer to a buffer */
  uintptr_t buffer;
  /* Number of bytes to be received */
  size_t rxLeft;
  /* Number of bytes to be transmitted */
  size_t txLeft;

  /* A transfer is in progress on the bus */
  bool active;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Transfer error flag */
  bool error;
  /* The active transfer is a reception */
  bool reading;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_I2C_H_ */