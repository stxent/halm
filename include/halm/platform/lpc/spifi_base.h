/*
 * halm/platform/lpc/spifi_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPIFI_BASE_H_
#define HALM_PLATFORM_LPC_SPIFI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SpifiBase;

struct SpifiBaseConfig
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
  /** Optional: use debug memory area for memory-mapped mode. */
  bool debug;
};

struct SpifiBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Use debug memory area */
  bool debug;
  /* Select quad or dual mode */
  bool wide;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
uint32_t spifiGetClock(const struct SpifiBase *);
void *spifiGetMemoryAddress(const struct SpifiBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SPIFI_BASE_H_ */
