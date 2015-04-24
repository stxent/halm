/*
 * platform/nxp/i2s_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_I2S_BASE_H_
#define PLATFORM_NXP_I2S_BASE_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const I2sBase;
/*----------------------------------------------------------------------------*/
struct I2sBaseConfig
{
  /** Mandatory: data rate. */
  uint32_t rate;

  struct
  {
    /** Optional: receive clock. */
    pin_t sck;
    /** Optional: receive word select. */
    pin_t ws;
    /** Optional: receive word select. */
    pin_t sda;
    /** Optional: master clock output. */
    pin_t mclk;
  } rx;

  struct
  {
    /** Optional: transmit clock. */
    pin_t sck;
    /** Optional: transmit word select. */
    pin_t ws;
    /** Optional: transmit word select. */
    pin_t sda;
    /** Optional: master clock output. */
    pin_t mclk;
  } tx;

  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct I2sBase
{
  struct Interface parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_I2S_BASE_H_ */
