/*
 * adc.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/adc.h>
#include <platform/nxp/adc_defs.h>
#include <string.h>
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
    .write = 0 /* TODO Replace zero methods with something else */
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Adc = &adcTable;
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configPtr)
{
  const struct AdcConfig * const config = configPtr;
  const struct AdcBaseConfig parentConfig = {
      .pin = config->pin
  };
  struct Adc *interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = AdcBase->init(object, &parentConfig)) != E_OK)
    return res;

  interface->callback = 0;
  interface->blocking = true;
  interface->buffer = 0;
  interface->left = 0;

//  irqEnable(ADC_IRQ);
//  /* Enable global interrupt */
//  LPC_ADC->INTEN = INTEN_ADG;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  struct Adc *interface = object;

  AdcBase->deinit(interface); /* Call base class destructor */
}
/*----------------------------------------------------------------------------*/
static enum result adcCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Adc *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcGet(void *object, enum ifOption option, void *data)
{
  struct Adc *interface = object;

  switch (option)
  {
    case IF_READY:
      return E_OK; //TODO
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSet(void *object, enum ifOption option, const void *data)
{
  struct Adc *interface = object;

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
  struct Adc *interface = object;

  LPC_ADC->CR &= ~CR_SEL_MASK;
  //FIXME Magic
  LPC_ADC->CR |= CR_START(1) | CR_SEL(interface->parent.channel);

  while (!(LPC_ADC->GDR & GDR_DONE));

  LPC_ADC->CR &= ~CR_START_MASK; /* Stop conversion */
  if (LPC_ADC->GDR & GDR_OVERRUN) /* Return error when overrun occurred */
    return 0;

  uint16_t value = GDR_RESULT_VALUE(LPC_ADC->GDR, 10); //FIXME Magic values
  memcpy(buffer, &value, sizeof(value));

  return length;
}
