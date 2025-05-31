/*
 * halm/platform/stm32/bxcan_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_BXCAN_BASE_H_
#define HALM_PLATFORM_STM32_BXCAN_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/bxcan_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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

  /* Interrupt identifiers */
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
#endif /* HALM_PLATFORM_STM32_BXCAN_BASE_H_ */
