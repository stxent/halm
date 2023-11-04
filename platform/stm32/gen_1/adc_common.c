/*
 * adc_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/adc_base.h>
#include <halm/platform/stm32/gen_1/adc_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry adcPinGroups[];
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *interface, PinNumber key)
{
  const struct PinGroupEntry * const group = pinGroupFind(adcPinGroups, key,
      interface->channel);
  assert(group != NULL);

  const uint8_t currentPinNumber = PIN_TO_OFFSET(key);
  const uint8_t firstPinNumber = PIN_TO_OFFSET(group->begin);
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, PIN_ANALOG);

  return (struct AdcPin){group->value + currentPinNumber - firstPinNumber};
}
/*----------------------------------------------------------------------------*/
uint32_t adcEncodeSensitivity(enum InputEvent event)
{
#ifndef CONFIG_PLATFORM_STM32_ADC_BASIC
  switch (event)
  {
    case INPUT_RISING:
      return EXTEN_RISING;

    case INPUT_FALLING:
      return EXTEN_FALLING;

    case INPUT_TOGGLE:
      return EXTEN_TOGGLE;

    default:
      return EXTEN_DISABLED;
  }
#else
  (void)event;
  return 0;
#endif
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void adcSetupPins(struct AdcBase *interface, const PinNumber *pins,
    struct AdcPin *output, size_t count)
{
  for (size_t index = 0; index < count; ++index)
    *output++ = adcConfigPin(interface, pins[index]);
}
