/*
 * gptimer_common.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gptimer_base.h>
#include <halm/platform/lpc/gptimer_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpTimerCapturePins[];
extern const struct PinEntry gpTimerMatchPins[];
/*----------------------------------------------------------------------------*/
int gpTimerAllocateChannel(uint8_t mask)
{
  int pos = 4; /* Each timer has 4 match blocks */
  while (--pos >= 0 && (mask & (1 << pos)));
  return pos;
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigCapturePin(uint8_t channel, PinNumber key,
    enum PinPull pull)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerCapturePins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);
  const uint8_t number = UNPACK_CHANNEL(pinEntry->value);

  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
  pinSetPull(pin, pull);

  gpTimerEnableCapture(channel, number);
  return number;
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigMatchPin(uint8_t channel, PinNumber key, bool value)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerMatchPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, value);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerGetMatchChannel(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerMatchPins, key, channel);
  assert(pinEntry != NULL);

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void gpTimerSetFrequency(struct GpTimerBase *timer, uint32_t frequency)
{
  LPC_TIMER_Type * const reg = timer->reg;

  if (frequency)
  {
    const uint32_t apbClock = gpTimerGetClock(timer);
    const uint32_t divisor = apbClock / frequency - 1;

    assert(frequency <= apbClock);
    assert(divisor <= gpTimerGetMaxValue(timer));

    reg->PR = divisor;
  }
  else
    reg->PR = 0;
}
