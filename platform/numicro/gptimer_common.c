/*
 * gptimer_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/gptimer_base.h>
#include <halm/platform/numicro/gptimer_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpTimerPins[];
/*----------------------------------------------------------------------------*/
void gpTimerConfigPin(uint8_t channel, PinNumber key, enum PinPull pull)
{
  const struct PinEntry * const pinEntry = pinFind(gpTimerPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);
  pinSetPull(pin, pull);
}
/*----------------------------------------------------------------------------*/
void gpTimerSetTimerFrequency(struct GpTimerBase *timer, uint32_t frequency)
{
  NM_TIMER_Type * const reg = timer->reg;
  uint32_t divisor;

  if (frequency)
  {
    const uint32_t apbClock = gpTimerGetClock(timer);
    assert(frequency <= apbClock);

    divisor = apbClock / frequency - 1;
    assert(divisor <= MASK(CTL_PSC_WIDTH));
  }
  else
    divisor = 0;

  reg->CTL = (reg->CTL & ~CTL_PSC_MASK) | CTL_PSC(divisor);
}
