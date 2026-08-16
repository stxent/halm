/*
 * halm/platform/stm32/fsmc_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FSMC_BASE_H_
#define HALM_PLATFORM_STM32_FSMC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
struct FsmcPinDescription
{
  PinNumber clk;
  PinNumber nl;
  PinNumber noe;
  PinNumber nwe;
  PinNumber nwait;

  PinNumber intr[3]; /* FSMC_INT2, FSMC_INT3 and FSMC_INTR */
  PinNumber nbl[2];
  PinNumber ne[4];
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t fsmcGetClock(void);
void *fsmcGetMemoryAddress(uint8_t, uint8_t);
bool fsmcSetMemoryDescriptor(uint8_t, const struct Entity *, struct Entity *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_FSMC_BASE_H_ */
