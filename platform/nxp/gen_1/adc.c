/*
 * adc.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/adc.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcSetCallback(void *, void (*)(void *), void *);
static enum result adcGetParam(void *, enum IfParameter, void *);
static enum result adcSetParam(void *, enum IfParameter, const void *);
static size_t adcRead(void *, void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct Adc),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Adc = &adcTable;
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
  assert(config);

  struct Adc * const interface = object;

  /* Initialize input pin */
  adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);

  interface->unit = config->parent;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  const struct Adc * const interface = object;

  adcReleasePin(interface->pin);
}
/*----------------------------------------------------------------------------*/
static enum result adcSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result adcGetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result adcSetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct Adc * const interface = object;
  LPC_ADC_Type * const reg = interface->unit->base.reg;
  const uint8_t channel = interface->pin.channel;

  if (length < SAMPLE_SIZE)
    return 0;

  if (adcUnitRegister(interface->unit, 0, interface) != E_OK)
    return 0;

  /* Set conversion channel */
  reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL_CHANNEL(channel);

  /* Perform a new conversion */
  uint32_t value;

  reg->CR |= CR_START(ADC_SOFTWARE);

  do
  {
    value = reg->DR[channel];
  }
  while (!(value & DR_DONE));

  reg->CR &= ~CR_START_MASK;

  /* Copy a result into the first element of the array */
  *((uint16_t *)buffer) = DR_RESULT_VALUE(value);

  adcUnitUnregister(interface->unit);

  return SAMPLE_SIZE;
}
