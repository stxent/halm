/*
 * halm/platform/lpc/i2c.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_I2C_H_
#define HALM_PLATFORM_LPC_I2C_H_
/*----------------------------------------------------------------------------*/
#include <halm/target.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_I2C/i2c_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
enum I2CParameter
{
  /**
   * Send stop after write operation. Option controls the repeated start
   * condition generation. This option is enabled by default.
   */
  IF_I2C_SENDSTOP = IF_PARAMETER_END
};
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

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Pointer to an output buffer */
  const uint8_t *txBuffer;
  /* Bytes left for transmission or reception */
  uint16_t rxLeft, txLeft;

  /* Address of the device, only 7-bit addressing is supported */
  uint8_t address;
  /* Current interface state */
  uint8_t state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Generate stop condition after writing */
  bool sendStopBit;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_I2C_H_ */
