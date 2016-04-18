/*
 * platform/nxp/gen_1/uart_base.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_GEN_1_UART_BASE_H_
#define HALM_PLATFORM_NXP_GEN_1_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UartBase;
/*----------------------------------------------------------------------------*/
enum uartParity
{
  UART_PARITY_NONE,
  UART_PARITY_ODD,
  UART_PARITY_EVEN
};
/*----------------------------------------------------------------------------*/
struct UartRateConfig
{
  uint16_t divisor;
  uint8_t fraction;
};
/*----------------------------------------------------------------------------*/
struct UartBaseConfig
{
  /** Mandatory: serial input. */
  pinNumber rx;
  /** Mandatory: serial output. */
  pinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UartBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
enum result uartCalcRate(const struct UartBase *, uint32_t,
    struct UartRateConfig *);
void uartConfigPins(struct UartBase *, const struct UartBaseConfig *);
uint32_t uartGetRate(const struct UartBase *);
void uartSetParity(struct UartBase *, enum uartParity);
void uartSetRate(struct UartBase *, struct UartRateConfig);

uint32_t uartGetClock(const struct UartBase *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_GEN_1_UART_BASE_H_ */
