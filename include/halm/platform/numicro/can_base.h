/*
 * halm/platform/numicro/can_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_CAN_BASE_H_
#define HALM_PLATFORM_NUMICRO_CAN_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const CanBase;

struct CanBaseConfig
{
  /** Mandatory: receiver input. */
  PinNumber rx;
  /** Mandatory: transmitter output. */
  PinNumber tx;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct CanBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t canGetClock(const struct CanBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_CAN_BASE_H_ */
