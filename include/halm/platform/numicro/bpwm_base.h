/*
 * halm/platform/numicro/bpwm_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_BPWM_BASE_H_
#define HALM_PLATFORM_NUMICRO_BPWM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const BpwmUnitBase;

struct BpwmUnitBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct BpwmUnitBase
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

/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
uint8_t bpwmConfigPin(uint8_t, PinNumber);
void bpwmSetFrequency(struct BpwmUnitBase *, uint32_t);

/* Platform-specific functions */
uint32_t bpwmGetClock(const struct BpwmUnitBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_BPWM_BASE_H_ */
