/*
 * platform/nxp/ssp_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SSP_BASE_H_
#define PLATFORM_NXP_SSP_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SspBase;
/*----------------------------------------------------------------------------*/
struct SspBaseConfig
{
  /** Optional: slave select pin. Available in slave mode only. */
  pinNumber cs;
  /**
   * Optional: pin acts as data input in master mode and as data output
   * in slave mode.
   */
  pinNumber miso;
  /**
   * Optional: pin acts as serial data output in master mode and
   * as data input in slave mode.
   */
  pinNumber mosi;
  /**
   * Optional: serial clock output for masters and input for slaves.
   * May be left unused in specific emulation modes.
   */
  pinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct SspBase
{
  struct Interface base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
void sspConfigPins(struct SspBase *, const struct SspBaseConfig *);
uint32_t sspGetRate(const struct SspBase *);
void sspSetRate(struct SspBase *, uint32_t);

uint32_t sspGetClock(const struct SspBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SSP_BASE_H_ */
