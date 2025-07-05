/*
 * halm/platform/lpc/gen_1/i2c.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_I2C_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_I2C_H_
#define HALM_PLATFORM_LPC_GEN_1_I2C_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/i2c_base.h>
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
};

struct I2C
{
  struct I2CBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* Pointer to an input or output buffer */
  uintptr_t buffer;
  /* Bytes to be received */
  size_t rxLeft;
  /* Bytes to be transmitted */
  size_t txLeft;

  /* Address of the device, only 7-bit addresses are supported */
  uint8_t address;
  /* Current interface state */
  uint8_t state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Generate repeated start condition after a next transfer */
  bool sendRepeatedStart;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_I2C_H_ */
