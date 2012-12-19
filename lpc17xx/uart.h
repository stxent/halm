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
/*----------------------------------------------------------------------------*/
struct UartClass;
/*----------------------------------------------------------------------------*/
extern const struct UartClass *Uart;
/*----------------------------------------------------------------------------*/
struct UartConfig
{
  uint8_t channel; /* Peripheral number */
  gpioKey rx, tx; /* RX and TX pins */
  uint8_t priority; /* Interrupt priority */
};
/*----------------------------------------------------------------------------*/
struct UartConfigRate
{
  uint8_t high;
  uint8_t low;
  uint8_t fraction;
};
/*----------------------------------------------------------------------------*/
struct UartClass
{
  struct InterfaceClass parent;

  /* Interrupt handler, arguments: UART descriptor assigned to peripheral */
  void (*handler)(void *);
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;

  uint8_t channel; /* Peripheral number */
  struct Gpio rxPin, txPin; /* Peripheral pins */

  LPC_UART_TypeDef *reg; /* Pointer to UART registers */
  IRQn_Type irq; /* IRQ identifier */
};
/*----------------------------------------------------------------------------*/
enum result uartSetDescriptor(uint8_t, void *);
struct UartConfigRate uartCalcRate(uint32_t);
enum result uartSetRate(struct Uart *, struct UartConfigRate);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
