/*
 * gptimer_common.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/stm/gptimer_base.h>
#include <halm/platform/stm/gptimer_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpTimerCaptureComparePins[];
/*----------------------------------------------------------------------------*/
int gpTimerAllocateChannel(uint8_t mask)
{
  int pos = 4; /* Each timer has 4 match blocks */
  while (--pos >= 0 && (mask & (1 << pos)));
  return pos;
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigComparePin(uint8_t channel, PinNumber key)
{
  const struct PinEntry *pinEntry;
  size_t index = 0;

  for (; index < CHANNEL_COUNT; ++index)
  {
    pinEntry = pinFind(gpTimerCaptureComparePins, key,
        PACK_CHANNEL(channel, index));

    if (pinEntry)
    {
      const struct Pin pin = pinInit(key);

      pinOutput(pin, false);
      pinSetFunction(pin, pinEntry->value);
      break;
    }
  }

  assert(pinEntry);
  return UNPACK_CHANNEL(index) >> 1;
}
