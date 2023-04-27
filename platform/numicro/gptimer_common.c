/*
 * gptimer_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/gptimer_base.h>
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
