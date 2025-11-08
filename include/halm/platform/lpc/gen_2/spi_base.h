/*
 * halm/platform/lpc/gen_2/spi_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SPI_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_SPI_BASE_H_
#define HALM_PLATFORM_LPC_GEN_2_SPI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
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
   * Optional: serial clock output for masters and clock input for slaves.
   * May be left unused in special cases.
   */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
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
void spiConfigPins(const struct SpiBaseConfig *, const struct PinEntry *);
uint8_t spiGetMode(const struct SpiBase *);
uint32_t spiGetRate(const struct SpiBase *);
void spiSetMode(struct SpiBase *, uint8_t);
bool spiSetRate(struct SpiBase *, uint32_t);

/* Platform-specific functions */
uint32_t spiGetClock(const struct SpiBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_SPI_BASE_H_ */
