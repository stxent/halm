/*
 * platform/nxp/ssp_base.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef SSP_BASE_H_
#define SSP_BASE_H_
/*----------------------------------------------------------------------------*/
#include <gpio.h>
#include <interface.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SspBase;
/*----------------------------------------------------------------------------*/
/* TODO Add master/slave select */
struct SspBaseConfig
{
  /** Optional: slave select pin. Available in slave mode only. */
  gpio_t cs;
  /**
   * Optional: pin acts as data input in master mode and as data output
   * in slave mode.
   */
  gpio_t miso;
  /**
   * Optional: pin acts as serial data output in master mode and
   * as data input in slave mode.
   */
  gpio_t mosi;
  /**
   * Optional: serial clock output for masters and input for slaves.
   * May be left unused in specific emulation modes.
   */
  gpio_t sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct SspBase
{
  struct Interface parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
uint32_t sspGetRate(const struct SspBase *);
enum result sspSetRate(struct SspBase *, uint32_t);

uint32_t sspGetClock(const struct SspBase *);
enum result sspSetupPins(struct SspBase *, const struct SspBaseConfig *);
/*----------------------------------------------------------------------------*/
#endif /* SSP_BASE_H_ */
