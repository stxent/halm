/*
 * bpwm_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/bpwm_base.h>
#include <halm/platform/numicro/bpwm_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinEntry bpwmPins[];
/*----------------------------------------------------------------------------*/
uint8_t bpwmConfigPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(bpwmPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void bpwmSetFrequency(struct BpwmUnitBase *unit, uint32_t frequency)
{
  NM_BPWM_Type * const reg = unit->reg;

  if (frequency)
  {
    // TODO Clocks from timers 0..3
    const uint32_t clock = bpwmGetClock(unit);
    const uint32_t prescaler = clock / frequency - 1;

    assert(prescaler <= CLKPSC_MAX);

    reg->CLKPSC = prescaler;
  }
  else
    reg->CLKPSC = 0;
}
