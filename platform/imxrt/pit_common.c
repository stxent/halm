/*
 * pit_common.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/pit_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry pitPins[];
/*----------------------------------------------------------------------------*/
void pitConfigTriggerPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(pitPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);
}
