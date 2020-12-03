/*
 * halm/platform/lpc/i2s_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_I2S_BASE_H_
#define HALM_PLATFORM_LPC_I2S_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum I2SWidth
{
  I2S_WIDTH_8,
  I2S_WIDTH_16,
  I2S_WIDTH_32
};

struct I2SRateConfig
{
  uint8_t x;
  uint8_t y;
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const I2SBase;

struct I2SBaseConfig
{
  struct
  {
    /** Optional: receive clock. */
    PinNumber sck;
    /** Optional: receive word select. */
    PinNumber ws;
    /** Optional: receive data. */
    PinNumber sda;
    /** Optional: master clock output. */
    PinNumber mclk;
  } rx;

  struct
  {
    /** Optional: transmit clock. */
    PinNumber sck;
    /** Optional: transmit word select. */
    PinNumber ws;
    /** Optional: transmit data. */
    PinNumber sda;
    /** Optional: master clock output. */
    PinNumber mclk;
  } tx;

  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct I2SBase
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
enum Result i2sCalcRate(struct I2SBase *, uint32_t, struct I2SRateConfig *);

/* Platform-specific functions */
uint32_t i2sGetClock(const struct I2SBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_I2S_BASE_H_ */
