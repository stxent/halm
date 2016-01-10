/*
 * adc_burst.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <platform/nxp/adc_burst.h>
#include <platform/nxp/gen_1/adc_defs.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(*((struct AdcBurst *)0)->buffer)
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcCallback(void *, void (*)(void *), void *);
static enum result adcGet(void *, enum ifOption, void *);
static enum result adcSet(void *, enum ifOption, const void *);
static uint32_t adcRead(void *, uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = sizeof(struct AdcBurst),
    .init = adcInit,
    .deinit = adcDeinit,

    .callback = adcCallback,
    .get = adcGet,
    .set = adcSet,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcBurst = &adcTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcBurst * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;

  /* Copy conversion result and increase position in buffer */
  *interface->buffer = DR_RESULT_VALUE(reg->DR[interface->pin.channel]);
  ++interface->buffer;

  if (!--interface->left)
  {
    /* Stop automatic conversion */
    reg->CR &= ~CR_START_MASK;
    /* Disable interrupts */
    reg->INTEN = 0;
    irqDisable(unit->base.irq);

    adcUnitUnregister(unit);

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
  else
    reg->CR = reg->CR;
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configBase)
{
  const struct AdcBurstConfig * const config = configBase;
  struct AdcBurst * const interface = object;
  enum result res;

  assert(config->event < ADC_EVENT_END);

  /* Initialize input pin */
  res = adcConfigPin((struct AdcUnitBase *)config->parent, config->pin,
      &interface->pin);
  if (res != E_OK)
    return res;

  interface->callback = 0;
  interface->blocking = true;
  interface->buffer = 0;
  interface->left = 0;
  interface->unit = config->parent;
  /* Convert enumerator constant to register value */
  interface->event = 1 + config->event;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  const struct AdcBurst * const interface = object;

  adcReleasePin(interface->pin);
}
/*----------------------------------------------------------------------------*/
static enum result adcCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcBurst * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcGet(void *object, enum ifOption option, void *data)
{
  struct AdcBurst * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      return interface->left ? E_BUSY : E_OK;

    case IF_WIDTH:
      *((uint32_t *)data) = ADC_RESOLUTION;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSet(void *object, enum ifOption option,
    const void *data __attribute__((unused)))
{
  struct AdcBurst * const interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t adcRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct AdcBurst * const interface = object;
  struct AdcUnit * const unit = interface->unit;
  LPC_ADC_Type * const reg = unit->base.reg;
  const uint32_t samples = length / SAMPLE_SIZE;

  if (!samples)
    return 0;

  if (adcUnitRegister(unit, interruptHandler, interface) != E_OK)
    return 0;

  /* Enable interrupt after completion of the conversion */
  irqEnable(unit->base.irq);
  reg->INTEN = INTEN_AD(interface->pin.channel);

  /* Set conversion channel */
  reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL(interface->pin.channel);

  interface->buffer = (void *)buffer;
  interface->left = samples;

  /* Start the conversion */
  reg->CR |= CR_START(interface->event);

  if (interface->blocking)
  {
    while (interface->left)
      barrier();
  }

  return samples * SAMPLE_SIZE;
}
