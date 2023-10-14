/*
 * halm/platform/stm32/spi_base.h
 * Copyright (C) 2018, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SPI_BASE_H_
#define HALM_PLATFORM_STM32_SPI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/spi_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
void spiConfigPins(const struct SpiBaseConfig *);
uint8_t spiGetMode(const struct SpiBase *);
uint32_t spiGetRate(const struct SpiBase *);
void spiSetMode(struct SpiBase *, uint8_t);
void spiSetRate(struct SpiBase *, uint32_t);

/* Platform-specific functions */
uint32_t spiGetClock(const struct SpiBase *);
void *spiMakeOneShotDma(uint8_t, uint8_t, enum DmaPriority, enum DmaType);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SPI_BASE_H_ */
