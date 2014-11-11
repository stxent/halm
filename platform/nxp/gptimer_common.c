/*
 * gptimer_common.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/gptimer_base.h>
#include <platform/nxp/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpTimerCapturePins[];
extern const struct PinEntry gpTimerMatchPins[];
/*----------------------------------------------------------------------------*/
int8_t gpTimerAllocateChannel(uint8_t used)
{
  int8_t pos = 4; /* Each timer has 4 match blocks */

  while (--pos >= 0 && used & (1 << pos));

  return pos;
}
/*----------------------------------------------------------------------------*/
int8_t gpTimerConfigCapturePin(uint8_t channel, pin_t key, enum pinPull pull)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerCapturePins, key, channel);

  if (!pinEntry)
    return -1;

  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
  pinSetPull(pin, pull);

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
int8_t gpTimerConfigMatchPin(uint8_t channel, pin_t key)
{
  const struct PinEntry * const pinEntry =
      pinFind(gpTimerMatchPins, key, channel);

  if (!pinEntry)
    return -1;

  const struct Pin pin = pinInit(key);
  pinOutput(pin, 0);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
