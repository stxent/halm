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
extern const struct GpioDescriptor gpTimerCapturePins[];
extern const struct GpioDescriptor gpTimerMatchPins[];
/*----------------------------------------------------------------------------*/
int8_t gpTimerAllocateChannel(uint8_t used)
{
  int8_t pos = 4; /* Each timer has 4 match blocks */

  while (--pos >= 0 && used & (1 << pos));

  return pos;
}
/*----------------------------------------------------------------------------*/
int8_t gpTimerSetupCapturePin(uint8_t channel, gpio_t key)
{
  const struct GpioDescriptor *pinDescriptor;

  if (!(pinDescriptor = gpioFind(gpTimerCapturePins, key, channel)))
    return -1;

  const struct Gpio pin = gpioInit(key);
  gpioInput(pin);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  return UNPACK_CHANNEL(pinDescriptor->value);
}
/*----------------------------------------------------------------------------*/
int8_t gpTimerSetupMatchPin(uint8_t channel, gpio_t key)
{
  const struct GpioDescriptor *pinDescriptor;

  if (!(pinDescriptor = gpioFind(gpTimerMatchPins, key, channel)))
    return -1;

  const struct Gpio pin = gpioInit(key);
  gpioOutput(pin, 0);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  return UNPACK_CHANNEL(pinDescriptor->value);
}
