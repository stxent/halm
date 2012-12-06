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
#include "queue.h"
#include "mutex.h"
/*----------------------------------------------------------------------------*/
struct Uart;
struct UartClass;
/*----------------------------------------------------------------------------*/
extern const struct UartClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  uint8_t channel;
  gpioKey rx, tx;
  uint32_t rate;
  uint16_t rxLength, txLength; /* Queue lengths */
  uint8_t priority; /* Interrupt priority */
};
/*----------------------------------------------------------------------------*/
struct UartClass
{
  struct InterfaceClass parent;

  /* Interrupt handler, arguments: UART descriptor assigned to peripheral */
  void (*handler)(struct Uart *);
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;

  uint8_t channel; /* Peripheral number */
  struct Gpio rxPin, txPin; /* Peripheral pins */

  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */
  Mutex queueLock; /* Access to queues */

  /* Controller-specific data */
  LPC_UART_TypeDef *reg;
  IRQn_Type irq;
};
/*----------------------------------------------------------------------------*/
enum result uartSetDescriptor(uint8_t, void *);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
