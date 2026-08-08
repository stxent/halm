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
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SpifiBase;

struct SpifiBaseConfig
{
  /** Mandatory: chip select pin. */
  PinNumber cs;
  /**
   * Mandatory: data output pin in single-wire mode, or input/output pin 0
   * in dual/quad-wire modes.
   */
  PinNumber io0;
  /**
   * Mandatory: data input pin in single-wire mode, or input/output pin 1
   * in dual/quad-wire modes.
   */
  PinNumber io1;
  /** Optional: input/output pin 2 in quad-wire mode. */
  PinNumber io2;
  /** Optional: input/output pin 3 in quad-wire mode. */
  PinNumber io3;
  /** Mandatory: serial clock output pin. */
  PinNumber sck;
  /** Optional: pin signaling slew rate. */
  enum PinSlewRate speed;
  /** Mandatory: hardware peripheral channel number. */
  uint8_t channel;
};

struct SpifiBase
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
uint32_t spifiGetClock(const struct SpifiBase *);
void *spifiGetMemoryAddress(const struct SpifiBase *, bool);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SPIFI_BASE_H_ */
