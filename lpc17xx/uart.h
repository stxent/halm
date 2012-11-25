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
#include "gpio.h"
#include "queue.h"
#include "mutex.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  gpioKey rx, tx;
  uint8_t channel;
  uint32_t rate;
  uint16_t rxLength, txLength; /* Queue lengths */
  uint8_t priority; /* Interrupt priority */
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;
  struct Mutex lock;
  struct Gpio rxPin, txPin;
  uint8_t channel;
  bool active;

  /* Receive and transmit buffers */
  struct Queue rxQueue, txQueue;

  /* Device-specific data */
  LPC_UART_TypeDef *reg;
  IRQn_Type irq;
};
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
