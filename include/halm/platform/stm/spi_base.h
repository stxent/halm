/*
 * halm/platform/stm/spi_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_SPI_BASE_H_
#define HALM_PLATFORM_STM_SPI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SpiBase;

struct SpiBaseConfig
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
  /** Mandatory: enable slave mode. */
  bool slave;
};

struct SpiBase
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
void spiConfigPins(struct SpiBase *, const struct SpiBaseConfig *);
uint32_t spiGetRate(const struct SpiBase *);
void spiSetRate(struct SpiBase *, uint32_t);

/* Platform-specific functions */
uint32_t spiGetClock(const struct SpiBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_SPI_BASE_H_ */
