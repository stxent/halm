/*
 * adc_oneshot.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <halm/platform/nxp/adc_oneshot.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
static uint16_t makeChannelConversion(struct AdcOneShot *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, enum IfParameter, void *);
static enum Result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct AdcOneShot),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcOneShot = &adcTable;
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
      .channel = config->channel
  };
  struct AdcOneShot * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);

  if (res == E_OK)
  {
    /* Enable analog function on the input pin */
    adcConfigPin(&interface->base, config->pin, &interface->pin);
    /* Calculate Control register value */
    interface->base.control |= CR_START(ADC_SOFTWARE)
        | CR_SEL_CHANNEL(interface->pin.channel);
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcOneShot * const interface = object;

  adcReleasePin(interface->pin);
  if (AdcBase->deinit)
    AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  /* Ensure proper alignment of the output buffer */
  assert(!((uintptr_t)buffer & 1));

  if (length < sizeof(uint16_t))
    return 0;

  struct AdcOneShot * const interface = object;

  if (adcSetInstance(interface->base.channel, &interface->base))
  {
    *(uint16_t *)buffer = makeChannelConversion(interface);
    adcResetInstance(interface->base.channel);
    return sizeof(uint16_t);
  }
  else
    return 0;
}
