/*
 * halm/platform/lpc/ssp_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_SSP_BASE_H_
#define HALM_PLATFORM_LPC_SSP_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SspBase;

struct SspBaseConfig
{
  /** Optional: slave select pin. Available in slave mode only. */
  PinNumber cs;
  /**
   * Optional: pin acts as data input in master mode and as data output
   * in slave mode.
   */
  PinNumber miso;
  /**
   * Optional: pin acts as serial data output in master mode and
   * as data input in slave mode.
   */
  PinNumber mosi;
  /**
   * Optional: serial clock output for masters and input for slaves.
   * May be left unused in specific emulation modes.
   */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct SspBase
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
void sspConfigPins(struct SspBase *, const struct SspBaseConfig *);
uint32_t sspGetRate(const struct SspBase *);
void sspSetRate(struct SspBase *, uint32_t);

/* Platform-specific functions */
uint32_t sspGetClock(const struct SspBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SSP_BASE_H_ */
