/*
 * gptimer_common.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gptimer_base.h>
#include <halm/platform/nxp/gptimer_defs.h>
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
uint8_t gpTimerConfigCapturePin(uint8_t channel, pinNumber key,
    enum pinPull pull)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerCapturePins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
  pinSetPull(pin, pull);

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigMatchPin(uint8_t channel, pinNumber key)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerMatchPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
