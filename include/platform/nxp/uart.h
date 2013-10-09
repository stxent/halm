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
  gpioKey rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
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

  /* Pointer to the UART register block */
  void *reg;
  /* Pointer to the interrupt handler */
  void (*handler)(void *);
  /* Interrupt identifier */
  irq_t irq;

  /* Interface pins */
  struct Gpio rxPin, txPin;
  /* Peripheral block identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UartRateConfig uartCalcRate(struct Uart *, uint32_t);
uint32_t uartGetRate(struct Uart *);
void uartSetParity(struct Uart *, enum uartParity);
void uartSetRate(struct Uart *, struct UartRateConfig);
/*----------------------------------------------------------------------------*/
extern uint32_t uartGetClock(struct Uart *);
extern enum result uartSetupPins(struct Uart *, const struct UartConfig *);
/*----------------------------------------------------------------------------*/
#endif /* UART_H_ */
