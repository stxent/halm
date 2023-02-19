/*
 * adc_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/adc_base.h>
#include <halm/platform/numicro/adc_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry adcPins[];
/*----------------------------------------------------------------------------*/
uint32_t adcMakePinCondition(enum PinEvent event)
{
  assert(event != PIN_TOGGLE);

  switch (event)
  {
    case PIN_RISING:
      return TRGCOND_RISING;

    case PIN_FALLING:
      return TRGCOND_RISING;

    case PIN_LOW:
      return TRGCOND_LOW;

    case PIN_HIGH:
      return TRGCOND_HIGH;

    default:
      break;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
uint32_t adcSetupPins(struct AdcBase *interface, const PinNumber *pins,
    struct AdcPin *output, size_t count)
{
  uint32_t enabled = 0;

  for (size_t index = 0; index < count; ++index)
  {
    *output = adcConfigPin(interface, pins[index]);

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    const unsigned int channel = output->channel;

    assert(!(enabled >> channel));
    enabled |= 1 << channel;

    ++output;
  }

  assert(enabled != 0);
  return enabled;
}
/*----------------------------------------------------------------------------*/
struct Pin adcSetupTriggerPin(uint8_t channel, PinNumber key)
{
  const struct PinEntry * const entry = pinFind(adcPins, key, channel);
  assert(entry);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, entry->value);

  return pin;
}