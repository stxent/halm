/*
 * adc_common.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
uint32_t adcSetupPins(const struct AdcBase *interface, struct AdcPin *channels,
    const PinNumber *pins, size_t count)
{
  uint32_t enabled = 0;

  for (size_t index = 0; index < count; ++index)
  {
    const struct AdcPin pin = adcConfigPin(interface, pins[index]);

    /*
     * Check whether the order of pins is correct and all pins
     * are unique. Pins must be sorted by analog channel number to ensure
     * direct connection between pins in the configuration
     * and an array of measured values.
     */
    assert(!(enabled >> pin.channel));

    channels[index] = pin;
    enabled |= 1 << pin.channel;
  }

  return enabled;
}
/*----------------------------------------------------------------------------*/
void adcStartCalibration(struct AdcBase *interface)
{
  assert(adcGetInstance(interface->sequence) == interface);

  LPC_ADC_Type * const reg = interface->reg;
  const uint32_t control = reg->CTRL;

  /* Reconfigure ADC clock */
  adcEnterCalibrationMode(interface);

  /* Start calibration */
  reg->CTRL = (reg->CTRL & ~CTRL_LPWRMODE) | CTRL_CALMODE;
  while (reg->CTRL & CTRL_CALMODE);

  /* Restore configuration */
  reg->CTRL = control;
}
