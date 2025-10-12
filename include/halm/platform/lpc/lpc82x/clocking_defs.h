/*
 * halm/platform/lpc/lpc82x/clocking_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_CLOCKING_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_CLOCKING_DEFS_H_
#define HALM_PLATFORM_LPC_LPC82X_CLOCKING_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
/*----------------------------------------------------------------------------*/
#define LPC_LPO_CLOCK
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void clockSourceUpdate(struct ClockDescriptor *descriptor)
{
  descriptor->sourceUpdate = 0;
  descriptor->sourceUpdate = CLKUEN_ENA;
}

static inline void configClockOutput(PinNumber key)
{
  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetMux(pin, PINMUX_CLKOUT);
}

static inline void configCrystalPins(bool bypass)
{
  static const PinNumber inKey = PIN(0, 8); /* XTALIN */
  static const PinNumber outKey = PIN(0, 9); /* XTALOUT */
  struct Pin pin;

  pin = pinInit(inKey);
  pinInput(pin);
  pinSetFunction(pin, 6);

  if (!bypass)
  {
    pin = pinInit(outKey);
    pinInput(pin);
    pinSetFunction(pin, 7);
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_CLOCKING_DEFS_H_ */
