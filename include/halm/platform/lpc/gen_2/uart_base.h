/*
 * halm/platform/lpc/gen_2/uart_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_UART_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_UART_BASE_H_
#define HALM_PLATFORM_LPC_GEN_2_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UartBase;

struct UartBaseConfig
{
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
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

/* Common functions */
void uartConfigPins(const struct UartBaseConfig *, const struct PinEntry *);
enum SerialParity uartGetParity(const struct UartBase *);
uint32_t uartGetRate(const struct UartBase *);
enum SerialStopBits uartGetStopBits(const struct UartBase *);
void uartSetParity(struct UartBase *, enum SerialParity);
bool uartSetRate(struct UartBase *, uint32_t);
void uartSetStopBits(struct UartBase *, enum SerialStopBits);

/* Platform-specific functions */
uint32_t uartGetClock(const struct UartBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_UART_BASE_H_ */
