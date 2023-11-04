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
uint8_t gpTimerConfigInputPin(uint8_t channel, PinNumber key,
    enum PinPull pull)
{
  size_t index = 0;

  for (; index < CHANNEL_COUNT; ++index)
  {
    const struct PinEntry * const pinEntry = pinFind(gpTimerPins, key,
        PACK_CHANNEL(channel, index));

    if (pinEntry != NULL)
    {
      const struct Pin pin = pinInit(key);

      pinInput(pin);
      pinSetPull(pin, pull);
      pinSetFunction(pin, pinEntry->value);
      break;
    }
  }

  assert(index != CHANNEL_COUNT);
  return index;
}
/*----------------------------------------------------------------------------*/
uint8_t gpTimerConfigOutputPin(uint8_t channel, PinNumber key)
{
  size_t index = 0;

  for (; index < CHANNEL_COUNT; ++index)
  {
    const struct PinEntry * const pinEntry = pinFind(gpTimerPins, key,
        PACK_CHANNEL(channel, index));

    if (pinEntry != NULL)
    {
      const struct Pin pin = pinInit(key);

      pinOutput(pin, false);
      pinSetFunction(pin, pinEntry->value);
      break;
    }
  }

  assert(index != CHANNEL_COUNT);
  return index;
}
/*----------------------------------------------------------------------------*/
void gpTimerSetTimerFrequency(struct GpTimerBase *timer, uint32_t frequency)
{
  STM_TIM_Type * const reg = timer->reg;
  uint32_t divisor;

  if (frequency)
  {
    const uint32_t apbClock = gpTimerGetClock(timer);
    assert(frequency <= apbClock);

    divisor = apbClock / frequency - 1;
    assert(divisor <= 0xFFFF);
  }
  else
    divisor = 0;

  reg->PSC = divisor;
  reg->EGR = EGR_UG;
}
