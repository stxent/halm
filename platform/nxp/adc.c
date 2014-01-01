/*
 * adc.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <string.h>
#include <platform/nxp/adc.h>
#include <platform/nxp/adc_defs.h>
/*----------------------------------------------------------------------------*/
#define RESULT_POW              1 /* Power of two */
#define UNPACK_FUNCTION(value)  ((value) & 0x0F) /* Unpack function */
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F) /* Unpack channel */
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *, const void *);
static void adcUnitDeinit(void *);
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
static enum result adcCallback(void *, void (*)(void *), void *);
static enum result adcGet(void *, enum ifOption, void *);
static enum result adcSet(void *, enum ifOption, const void *);
static uint32_t adcRead(void *, uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = sizeof(struct AdcUnit),
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
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
extern const struct GpioDescriptor adcPins[];
const struct EntityClass *AdcUnit = &adcUnitTable;
const struct InterfaceClass *Adc = &adcTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcUnit *unit = object;
  struct Adc *interface = unit->current;
  LPC_ADC_Type *reg = interface->unit->parent.reg;

  if (interface)
  {
    reg->CR &= ~CR_START_MASK;

    /* Copy conversion result */
    uint16_t value = GDR_RESULT_VALUE(reg->GDR, ADC_RESOLUTION);
    memcpy(interface->buffer, &value, sizeof(value));
    interface->buffer += 1 << RESULT_POW;

    if (!--interface->left)
    {
      /* Release converter lock when all conversions are done */
      unit->current = 0;
      spinUnlock(&unit->lock);
      if (interface->callback)
        interface->callback(interface->callbackArgument);
    }
    else
    {
      /* Start next conversion */
      reg->CR |= CR_START(interface->event);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configPtr)
{
  const struct AdcUnitConfig * const config = configPtr;
  const struct AdcUnitBaseConfig parentConfig = {
      .channel = config->channel
  };
  struct AdcUnit *unit = object;
  enum result res;

  /* Call base class constructor */
  if ((res = AdcUnitBase->init(object, &parentConfig)) != E_OK)
    return res;

  unit->current = 0;
  unit->lock = SPIN_UNLOCKED;
  unit->parent.handler = interruptHandler;

  /* Enable global conversion interrupt */
  ((LPC_ADC_Type *)unit->parent.reg)->INTEN = INTEN_ADG;
  /* Enable interrupt globally */
  irqEnable(unit->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  struct AdcUnit *unit = object;

  irqDisable(unit->parent.irq);
  AdcUnitBase->deinit(unit);
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configPtr)
{
  const struct AdcConfig * const config = configPtr;
  const struct GpioDescriptor *pinDescriptor;
  struct Adc *interface = object;

  assert(config->event < ADC_EVENT_END);

  if (!(pinDescriptor = gpioFind(adcPins, config->pin, 0)))
    return E_VALUE;

  /* Fill pin structure and initialize pin as input */
  struct Gpio pin = gpioInit(config->pin);
  gpioInput(pin);
  /* Enable analog pin mode bit */
  gpioSetFunction(pin, GPIO_ANALOG);
  /* Set analog pin function */
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  interface->callback = 0;
  interface->channel = UNPACK_CHANNEL(pinDescriptor->value);
  interface->blocking = true;
  interface->buffer = 0;
  interface->left = 0;
  interface->unit = config->parent;
  /* Convert enumerator constant to register value */
  interface->event = config->event + 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result adcCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Adc *interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result adcGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct Adc *interface = object;

  switch (option)
  {
    case IF_STATUS:
      return interface->left ? E_BUSY : E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcSet(void *object, enum ifOption option,
    const void *data __attribute__((unused)))
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
  LPC_ADC_Type *reg = interface->unit->parent.reg;

  /* Check buffer alignment */
  assert(!(length & ((1 << RESULT_POW) - 1)));

  if (!length || !spinTryLock(&interface->unit->lock))
    return 0;

  /* Set conversion channel */
  reg->CR = (reg->CR & ~CR_SEL_MASK) | CR_SEL(interface->channel);

  interface->buffer = buffer;
  interface->left = length >> RESULT_POW;
  interface->unit->current = object;

  /* Start the conversion */
  reg->CR |= CR_START(interface->event);

  if (interface->blocking)
    while (interface->left);

  return length;
}
