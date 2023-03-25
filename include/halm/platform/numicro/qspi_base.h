/*
 * halm/platform/numicro/qspi_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_QSPI_BASE_H_
#define HALM_PLATFORM_NUMICRO_QSPI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const QspiBase;

struct QspiBaseConfig
{
  /** Optional: slave select pin. */
  PinNumber cs;
  /**
   * Optional: data output in single-wire master mode, data input in
   * single-wire slave mode, input/output pin 0 in dual or quad mode.
   */
  PinNumber io0;
  /**
   * Optional: data input in single-wire master mode, data output in
   * single-wire slave mode, input/output pin 1 in dual or quad mode.
   */
  PinNumber io1;
  /** Optional: input/output pin 2 in quad mode. */
  PinNumber io2;
  /** Optional: input/output pin 3 in quad mode. */
  PinNumber io3;
  /** Optional: serial clock output for masters and clock input for slaves. */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct QspiBase
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
void qspiConfigPins(const struct QspiBaseConfig *);
uint8_t qspiGetMode(const struct QspiBase *);
uint32_t qspiGetRate(const struct QspiBase *);
void qspiSetMode(struct QspiBase *, uint8_t);
void qspiSetRate(struct QspiBase *, uint32_t);

/* Platform-specific functions */
uint32_t qspiGetClock(const struct QspiBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_QSPI_BASE_H_ */
