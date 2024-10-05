/*
 * halm/platform/imxrt/lpuart_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_LPUART_BASE_H_
#define HALM_PLATFORM_IMXRT_LPUART_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const LpUartBase;

struct LpUartBaseConfig
{
  /** Optional: serial input. */
  PinNumber rx;
  /** Optional: serial output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct LpUartBase
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
void uartConfigPins(const struct LpUartBaseConfig *,
    enum PinDaisyIndex, enum PinDaisyIndex);
enum SerialParity uartGetParity(const struct LpUartBase *);
uint32_t uartGetRate(const struct LpUartBase *);
enum SerialStopBits uartGetStopBits(const struct LpUartBase *);
void uartSetParity(struct LpUartBase *, enum SerialParity);
bool uartSetRate(struct LpUartBase *, uint32_t);
void uartSetStopBits(struct LpUartBase *, enum SerialStopBits);

/* Platform-specific functions */
uint32_t uartGetClock(const struct LpUartBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_LPUART_BASE_H_ */
