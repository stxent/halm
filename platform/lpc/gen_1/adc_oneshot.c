/*
 * adc_oneshot.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_oneshot.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
static void makeChannelConversion(struct AdcOneShot *, uint16_t *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcOneShot = &(const struct InterfaceClass){
    .size = sizeof(struct AdcOneShot),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = NULL,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = NULL
};
/*----------------------------------------------------------------------------*/
static void makeChannelConversion(struct AdcOneShot *interface,
    uint16_t *output)
{
  /* ADC should be locked to avoid simultaneous access */
  assert(adcGetInstance(interface->base.channel) == &interface->base);

  const struct AdcPin * const pins = interface->pins;
  LPC_ADC_Type * const reg = interface->base.reg;

  for (size_t index = 0; index < interface->count; ++index)
  {
    const unsigned int channel = pins[index].channel;
    uint32_t value;

    /* Reconfigure peripheral and start the conversion */
    reg->CR = interface->base.control | CR_SEL(1 << channel);

    do
    {
      value = reg->DR[channel];
    }
    while (!(value & DR_DONE));

    reg->CR = 0;
    output[index] = (uint16_t)value;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcOneShotConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = 0,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcOneShot * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->pins = malloc(sizeof(struct AdcPin) * count);
  if (interface->pins == NULL)
    return E_MEMORY;

  interface->count = (uint8_t)count;
  /* Calculate Control register value */
  interface->base.control |= CR_START(ADC_SOFTWARE);
  /* Initialize input pins */
  adcSetupPins(&interface->base, interface->pins, config->pins, count);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcOneShot * const interface = object;

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->pins);

  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *, int, void *)
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  struct AdcOneShot * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, &interface->base, NULL);
      return E_OK;

    default:
      break;
  }
#else
  (void)object;
  (void)parameter;
#endif

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer,
    [[maybe_unused]] size_t length)
{
  struct AdcOneShot * const interface = object;

  /* Ensure that the buffer has enough space */
  assert(length >= interface->count * sizeof(uint16_t));

  makeChannelConversion(interface, buffer);
  return interface->count * sizeof(uint16_t);
}
