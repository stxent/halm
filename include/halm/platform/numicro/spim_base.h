/*
 * halm/platform/numicro/spim_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SPIM_BASE_H_
#define HALM_PLATFORM_NUMICRO_SPIM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SpimBase;

struct SpimBaseConfig
{
  /** Mandatory: chip select pin. */
  PinNumber cs;
  /**
   * Mandatory: data output in single-wire mode, input/output pin 0
   * in dual or quad mode.
   */
  PinNumber io0;
  /**
   * Mandatory: data input in single-wire mode, input/output pin 1
   * in dual or quad mode.
   */
  PinNumber io1;
  /** Optional: input/output pin 2 in quad mode. */
  PinNumber io2;
  /** Optional: input/output pin 3 in quad mode. */
  PinNumber io3;
  /** Mandatory: serial clock output. */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct SpimBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Select quad or dual mode */
  bool wide;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint32_t spimGetClock(const struct SpimBase *);
void *spimGetMemoryAddress(const struct SpimBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SPIM_BASE_H_ */
