/*
 * halm/platform/stm32/uart_base.h
 * Copyright (C) 2016, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_UART_BASE_H_
#define HALM_PLATFORM_STM32_UART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/stm32/dma.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/uart_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
void uartConfigPins(const struct UartBaseConfig *);
enum SerialParity uartGetParity(const struct UartBase *);
uint32_t uartGetRate(const struct UartBase *);
enum SerialStopBits uartGetStopBits(const struct UartBase *);
void uartSetParity(struct UartBase *, enum SerialParity);
bool uartSetRate(struct UartBase *, uint32_t);
void uartSetStopBits(struct UartBase *, enum SerialStopBits);

/* Platform-specific functions */
uint32_t uartGetClock(const struct UartBase *);
void *uartMakeCircularDma(uint8_t, uint8_t, enum DmaPriority, enum DmaType);
void *uartMakeOneShotDma(uint8_t, uint8_t, enum DmaPriority, enum DmaType);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_UART_BASE_H_ */
