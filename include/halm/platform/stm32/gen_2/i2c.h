/*
 * halm/platform/stm32/gen_2/i2c.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_I2C_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_2_I2C_H_
#define HALM_PLATFORM_STM32_GEN_2_I2C_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/i2c_base.h>
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
  /** Mandatory: number of the RX DMA stream. */
  uint8_t rxDma;
  /** Mandatory: number of the TX DMA stream. */
  uint8_t txDma;
};

struct I2C
{
  struct I2CBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel for data reception */
  struct Dma *rxDma;
  /* DMA channel for data transmission */
  struct Dma *txDma;

  /* Desired baud rate */
  uint32_t rate;

  /* Address of the device */
  uint16_t address;
  /* Current interface status */
  uint8_t status;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Generate repeated start condition after a next transfer */
  bool sendRepeatedStart;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_I2C_H_ */
