/*
 * halm/platform/lpc/gen_1/dac_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_DAC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_DAC_BASE_H_
#define HALM_PLATFORM_LPC_GEN_1_DAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DacBase;

struct DacBaseConfig
{
  /** Mandatory: analog output. */
  PinNumber pin;
};

struct DacBase
{
  struct Interface base;

  void *reg;

  /* Output pin */
  PinNumber pin;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t dacGetClock(const struct DacBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_DAC_BASE_H_ */
