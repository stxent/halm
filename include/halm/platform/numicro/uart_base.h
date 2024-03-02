/*
 * halm/platform/numicro/uart_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_UART_BASE_H_
#define HALM_PLATFORM_NUMICRO_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum
{
  UART_DEPTH_1  = 1,
  UART_DEPTH_16 = 16
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UartBase;

struct UartBaseConfig
{
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
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
  /* FIFO depth */
  uint8_t depth;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void uartConfigPins(const struct UartBaseConfig *);
enum SerialParity uartGetParity(const struct UartBase *);
uint32_t uartGetRate(const struct UartBase *);
void uartSetParity(struct UartBase *, enum SerialParity);
bool uartSetRate(struct UartBase *, uint32_t);

/* Platform-specific functions */
uint32_t uartGetClock(const struct UartBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_UART_BASE_H_ */
