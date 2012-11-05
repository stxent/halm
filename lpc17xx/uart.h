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
#include "LPC17xx.h"
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "queue.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
extern const struct EntityClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  gpioKey rx;
  gpioKey tx;
  uint8_t channel;
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;
  LPC_UART_TypeDef *block;
  IRQn_Type irq;
  struct Queue sendQueue, receiveQueue;
  struct Gpio rxPin, txPin;
  uint8_t channel;
  bool active;
};
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
