/*
 * uart.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_H_
#define UART_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "LPC13xx.h"
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "gpio.h"
#include "mutex.h"
#include "queue.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  gpioKey rx, tx;
  uint16_t rxLength, txLength;
  uint8_t channel;
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;
  struct Mutex lock;
  struct Gpio rxPin, txPin;
  struct Queue sendQueue, receiveQueue;
  uint8_t channel;
  bool active;

  /* Device-specific data */
  LPC_UART_TypeDef *block;
  IRQn_Type irq;
};
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
