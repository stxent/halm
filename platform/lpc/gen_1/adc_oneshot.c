/*
 * adc_oneshot.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_oneshot.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static uint16_t makeChannelConversion(struct AdcOneShot *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcOneShot = &(const struct InterfaceClass){
    .size = sizeof(struct AdcOneShot),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = 0,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
static uint16_t makeChannelConversion(struct AdcOneShot *interface)
{
  LPC_ADC_Type * const reg = interface->base.reg;
  uint32_t value;

  /* Reconfigure peripheral and start the conversion */
  reg->CR = interface->base.control;

  do
  {
    value = reg->DR[interface->pin.channel];
  }
  while (!(value & DR_DONE));

  reg->CR = 0;

  return DR_RESULT_VALUE(value);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcOneShotConfig * const config = configBase;
  assert(config);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = 0,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcOneShot * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Enable analog function on the input pin */
  interface->pin = adcConfigPin(&interface->base, config->pin);
  /* Calculate Control register value */
  interface->base.control |= CR_START(ADC_SOFTWARE)
      | CR_SEL_CHANNEL(interface->pin.channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcOneShot * const interface = object;

  adcReleasePin(interface->pin);
  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)), void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  struct AdcOneShot * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, 0, object) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, 0);
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
static size_t adcRead(void *object, void *buffer, size_t length)
{
  /* Ensure that the buffer has enough space */
  assert(length >= sizeof(uint16_t));
  /* Suppress warning */
  (void)length;

  struct AdcOneShot * const interface = object;
  const uint16_t value = makeChannelConversion(interface);

  memcpy(buffer, &value, sizeof(value));
  return sizeof(uint16_t);
}
