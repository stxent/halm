/*
 * uart.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_H_
#define UART_H_
/*----------------------------------------------------------------------------*/
#include "interface.h"
#include "platform/gpio.h"
#include "./device_defs.h"
#include "./nvic.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *Uart;
/*----------------------------------------------------------------------------*/
enum uartParity
{
  UART_PARITY_NONE = 0,
  UART_PARITY_ODD,
  UART_PARITY_EVEN
};
/*----------------------------------------------------------------------------*/
/* TODO Stop bits, frame width, additional signals and modes */
struct UartConfig
{
  uint32_t rate; /* Mandatory: baud rate */
  gpioKey rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
  enum uartParity parity; /* Optional: even, odd or no parity */
};
/*----------------------------------------------------------------------------*/
struct UartRateConfig
{
  uint8_t high, low, fraction;
};
/*----------------------------------------------------------------------------*/
struct Uart
{
  struct Interface parent;

  LPC_UART_TypeDef *reg; /* Pointer to UART registers */
  irq_t irq; /* IRQ identifier */

  void (*handler)(void *); /* Interrupt handler */
  struct Gpio rxPin, txPin; /* Peripheral pins */
  uint8_t channel; /* Peripheral number */
};
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(struct UartRateConfig *, uint32_t);
void uartSetRate(struct Uart *, struct UartRateConfig);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
