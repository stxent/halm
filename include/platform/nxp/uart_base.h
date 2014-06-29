/*
 * platform/nxp/uart_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef UART_BASE_H_
#define UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UartBase;
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
  /** Mandatory: serial input. */
  pin_t rx;
  /** Mandatory: serial output. */
  pin_t tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UartRateConfig
{
  uint16_t divisor;
  uint8_t fraction;
};
/*----------------------------------------------------------------------------*/
struct UartBase
{
  struct Interface parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(const struct UartBase *, uint32_t,
    struct UartRateConfig *);
uint32_t uartGetRate(const struct UartBase *);
void uartSetParity(struct UartBase *, enum uartParity);
void uartSetRate(struct UartBase *, struct UartRateConfig);

uint32_t uartGetClock(const struct UartBase *);
enum result uartSetupPins(struct UartBase *, const struct UartBaseConfig *);
/*----------------------------------------------------------------------------*/
#endif /* UART_BASE_H_ */
