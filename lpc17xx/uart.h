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
struct Uart;
struct UartClass;
/*----------------------------------------------------------------------------*/
extern const struct UartClass *Uart;
extern struct Uart *uartDescriptors[];
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

  struct Gpio rxPin, txPin; /* Peripheral pins */
  uint8_t channel; /* Peripheral number */
  struct Queue rxQueue, txQueue; /* Receive and transmit buffers */

  Mutex queueLock; /* Access to queues mutex */

  /* Controller-specific data */
  LPC_UART_TypeDef *reg;
  IRQn_Type irq;
};
/*----------------------------------------------------------------------------*/
/* TODO */
/* bool uartSetDescriptor(struct Uart **, const struct Uart *); */
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
