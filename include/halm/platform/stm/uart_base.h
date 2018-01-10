/*
 * halm/platform/stm/uart_base.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_UART_BASE_H_
#define HALM_PLATFORM_STM_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
enum UartParity
{
  UART_PARITY_NONE,
  UART_PARITY_ODD,
  UART_PARITY_EVEN
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UartBase;

struct UartBaseConfig
{
  /** Mandatory: serial input. */
  PinNumber rx;
  /** Mandatory: serial output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct UartBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void uartConfigPins(struct UartBase *, const struct UartBaseConfig *);
void uartSetParity(struct UartBase *, enum UartParity);
void uartSetRate(struct UartBase *, uint32_t);

uint32_t uartGetClock(const struct UartBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_UART_BASE_H_ */