/*
 * halm/platform/lpc/lpc11xx/clocking_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_CLOCKING_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC11XX_CLOCKING_DEFS_H_
#define HALM_PLATFORM_LPC_LPC11XX_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define LPC_WDT_CLOCK
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void clockSourceUpdate(struct ClockDescriptor *descriptor)
{
  descriptor->sourceUpdate = 0;
  descriptor->sourceUpdate = CLKUEN_ENA;
}

static inline void configClockOutput(PinNumber key)
{
  assert(key == PIN(0, 1));

  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetFunction(pin, 1);
}

static inline void configCrystalPins(bool)
{
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC11XX_CLOCKING_DEFS_H_ */
