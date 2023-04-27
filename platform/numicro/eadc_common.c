/*
 * eadc_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/eadc_base.h>
#include <halm/platform/numicro/eadc_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinEntry eadcPins[];
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct EadcBase *interface, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(eadcPins, key,
      interface->channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, PIN_ANALOG);

  return (struct AdcPin){pinEntry->value};
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
uint32_t adcSetupPins(struct EadcBase *interface, const PinNumber *pins,
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
  const struct PinEntry * const pinEntry = pinFind(eadcPins, key, channel);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  return pin;
}
