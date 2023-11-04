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
struct AdcPin adcConfigPin(const struct AdcBase *interface, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(adcPins, key,
      interface->channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, PIN_ANALOG);

  return (struct AdcPin){pinEntry->value};
}
/*----------------------------------------------------------------------------*/
uint32_t adcMakePinCondition(enum InputEvent event)
{
  assert(event != INPUT_TOGGLE);

  switch (event)
  {
    case INPUT_RISING:
      return TRGCOND_RISING;

    case INPUT_FALLING:
      return TRGCOND_RISING;

    case INPUT_LOW:
      return TRGCOND_LOW;

    case INPUT_HIGH:
      return TRGCOND_HIGH;

    default:
      break;
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(struct AdcPin adcPin __attribute__((unused)))
{
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
  const struct PinEntry * const pinEntry = pinFind(adcPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  return pin;
}
