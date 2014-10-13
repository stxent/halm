/*
 * adc_common.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <platform/nxp/adc_base.h>
#include <platform/nxp/adc_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry adcPins[];
/*----------------------------------------------------------------------------*/
int8_t adcSetupPin(uint8_t channel, pin_t key)
{
  const struct PinGroupEntry *group;
  union PinData begin, current;

  if (!(group = pinGroupFind(adcPins, key, channel)))
    return -1;

  begin.key = ~group->begin;
  current.key = ~key;

  const uint8_t function = UNPACK_FUNCTION(group->value);
  const uint8_t index = current.offset - begin.offset
      + UNPACK_CHANNEL(group->value);

  /* Fill pin structure and initialize pin as input */
  const struct Pin pin = pinInit(key);
  pinInput(pin);
  /* Enable analog pin mode bit */
  pinSetFunction(pin, PIN_ANALOG);
  /* Set analog pin function */
  pinSetFunction(pin, function);

  return index;
}
