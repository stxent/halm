/*
 * gptimer_common.c
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/gptimer_base.h>
#include <halm/platform/stm32/gptimer_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry gpTimerPins[];
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
  // TODO
  (void)channel;
  (void)key;
  (void)pull;
  return (uint8_t)-1;
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigComparePin(uint8_t channel, PinNumber key)
{
  const struct PinEntry *pinEntry;
  size_t index = 0;

  for (; index < CHANNEL_COUNT; ++index)
  {
    pinEntry = pinFind(gpTimerPins, key, PACK_CHANNEL(channel, index));

    if (pinEntry != NULL)
    {
      const struct Pin pin = pinInit(key);

      pinOutput(pin, false);
      pinSetFunction(pin, pinEntry->value);
      break;
    }
  }

  assert(pinEntry != NULL);
  return UNPACK_CHANNEL(index) >> 1;
}
