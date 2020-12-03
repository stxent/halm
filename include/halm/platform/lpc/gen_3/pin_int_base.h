/*
 * halm/platform/lpc/gen_3/pin_int_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_GEN_3_PIN_INT_BASE_H_
#define HALM_PLATFORM_LPC_GEN_3_PIN_INT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const PinIntBase;

struct PinIntBaseConfig
{
  /** Mandatory: pin number. */
  uint8_t number;
  /** Mandatory: pin port. */
  uint8_t port;
};

struct PinIntBase
{
  struct Entity base;

  void (*handler)(void *);
  IrqNumber irq;

  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_3_PIN_INT_BASE_H_ */
