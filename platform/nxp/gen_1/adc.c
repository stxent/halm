/*
 * adc.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/adc.h>
#include <platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcCallback(void *, void (*)(void *), void *);
static enum result adcGet(void *, enum ifOption, void *);
static enum result adcSet(void *, enum ifOption, const void *);
static uint32_t adcRead(void *, uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct Adc),
    .init = adcInit,
    .deinit = adcDeinit,

    .callback = adcCallback,
    .get = adcGet,
    .set = adcSet,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Adc = &adcTable;
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configBase)
{
  const struct AdcConfig * const config = configBase;
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
static enum result adcCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result adcGet(void *object __attribute__((unused)),
    enum ifOption option, void *data)
{
  switch (option)
  {
    case IF_WIDTH:
      *(uint32_t *)data = ADC_RESOLUTION;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t adcRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Adc * const interface = object;
  LPC_ADC_Type * const reg = interface->unit->base.reg;
  const uint8_t channel = interface->pin.channel;

  if (length < SAMPLE_SIZE)
    return 0;

  if (adcUnitRegister(interface->unit, 0, interface) != E_OK)
    return 0;

  /* Disable interrupts */
  reg->INTEN = 0;
  /* Set conversion channel */
  reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL(channel);

  /* Perform a new conversion */
  reg->CR |= CR_START(1 + ADC_SOFTWARE);
  while (!(reg->DR[channel] & DR_DONE));
  reg->CR &= ~CR_START_MASK;

  /* Copy result into first element of the array */
  *((uint16_t *)buffer) = DR_RESULT_VALUE(reg->DR[channel]);

  adcUnitUnregister(interface->unit);

  return SAMPLE_SIZE;
}
