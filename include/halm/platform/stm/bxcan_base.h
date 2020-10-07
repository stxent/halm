/*
 * halm/platform/stm/bxcan_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_BXCAN_BASE_H_
#define HALM_PLATFORM_STM_BXCAN_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const BxCanBase;

struct BxCanBaseConfig
{
  /** Mandatory: receiver input. */
  PinNumber rx;
  /** Mandatory: transmitter output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct BxCanBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);

  struct
  {
    IrqNumber rx0;
    IrqNumber rx1;
    IrqNumber sce;
    IrqNumber tx;
  } irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t canGetClock(const struct BxCanBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_BXCAN_BASE_H_ */
