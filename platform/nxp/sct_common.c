/*
 * sct_common.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/sct_base.h>
#include <halm/platform/nxp/sct_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinEntry sctInputPins[];
extern const struct PinEntry sctOutputPins[];
/*----------------------------------------------------------------------------*/
uint8_t sctConfigInputPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(sctInputPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
uint8_t sctConfigOutputPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(sctOutputPins, key, channel);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);

  pinOutput(pin, false);
  pinSetFunction(pin, UNPACK_FUNCTION(pinEntry->value));

  return UNPACK_CHANNEL(pinEntry->value);
}
/*----------------------------------------------------------------------------*/
void sctSetFrequency(struct SctBase *timer, uint32_t frequency)
{
  const unsigned int part = timer->part == SCT_HIGH;
  LPC_SCT_Type * const reg = timer->reg;

  /* Counter reset is recommended by the user manual */
  const uint32_t value = (reg->CTRL_PART[part] & ~CTRL_PRE_MASK) | CTRL_CLRCTR;

  if (frequency)
  {
    /* TODO Check whether the clock is from internal source */
    const uint32_t apbClock = sctGetClock(timer);
    const uint16_t prescaler = apbClock / frequency - 1;

    assert(prescaler < 256);

    reg->CTRL_PART[part] = value | CTRL_PRE(prescaler);
  }
  else
    reg->CTRL_PART[part] = 0;
}
