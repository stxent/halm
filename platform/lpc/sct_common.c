/*
 * sct_common.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/sct_base.h>
#include <halm/platform/lpc/sct_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sctInputPins[];
extern const struct PinEntry sctOutputPins[];
/*----------------------------------------------------------------------------*/
uint8_t sctConfigInputPin(uint8_t channel, PinNumber key, enum PinPull pull)
{
  const struct PinEntry * const pinEntry = pinFind(sctInputPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));
  pinSetPull(pin, pull);

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint8_t sctConfigOutputPin(uint8_t channel, PinNumber key, bool value)
{
  const struct PinEntry * const pinEntry = pinFind(sctOutputPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, value);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint8_t sctGetOutputChannel(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(sctOutputPins, key, channel);
  assert(pinEntry != NULL);

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void sctSetFrequency(struct SctBase *timer, uint32_t frequency)
{
  const unsigned int part = timer->part == SCT_HIGH;
  LPC_SCT_Type * const reg = timer->reg;

  /* Counter reset is recommended by the user manual */
  const uint16_t ctrl = (reg->CTRL_PART[part] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    const uint32_t apbClock = sctGetClock(timer);
    const uint16_t prescaler = apbClock / frequency - 1;

    assert(prescaler < 256);

    reg->CTRL_PART[part] = ctrl | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[part] = ctrl;
}
