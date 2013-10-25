/*
 * platform/nxp/lpc13xx/uart_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_BASE_H_
#define UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
#include <irq.h>
#include "platform_defs.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *UartBase;
/*----------------------------------------------------------------------------*/
enum uartParity
{
  UART_PARITY_NONE = 0,
  UART_PARITY_ODD,
  UART_PARITY_EVEN
};
/*----------------------------------------------------------------------------*/
struct UartBaseConfig
{
  gpio_t rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
};
/*----------------------------------------------------------------------------*/
struct UartRateConfig
{
  uint8_t high, low, fraction;
};
/*----------------------------------------------------------------------------*/
struct UartBase
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
struct UartRateConfig uartCalcRate(struct UartBase *, uint32_t);
uint32_t uartGetRate(struct UartBase *);
void uartSetParity(struct UartBase *, enum uartParity);
void uartSetRate(struct UartBase *, struct UartRateConfig);
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(struct UartBase *);
enum result uartSetupPins(struct UartBase *, const struct UartBaseConfig *);
/*----------------------------------------------------------------------------*/
#endif /* UART_BASE_H_ */
