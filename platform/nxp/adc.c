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
/* Unpack function */
#define UNPACK_FUNCTION(value)          ((value) & 0x0F)
/* Unpack match channel */
#define UNPACK_CHANNEL(value)           (((value) >> 4) & 0x0F)
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
    .size = sizeof(struct AdcUnitBase),
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

  unit->lock = SPIN_UNLOCKED;
  unit->parent.handler = interruptHandler;

  /* Disable all interrupt sources */
  ((LPC_ADC_Type *)unit->parent.reg)->INTEN = 0;
  /* Enable interrupt globally */
  irqEnable(unit->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  struct AdcUnit *unit = object;

  irqDisable(unit->parent.irq);
  AdcUnitBase->deinit(unit); /* Call base class destructor */
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configPtr)
{
  const struct AdcConfig * const config = configPtr;
  const struct GpioDescriptor *pinDescriptor;
  struct Adc *interface = object;

  //TODO ADC: add assertion for hardware event

  if (!(pinDescriptor = gpioFind(adcPins, config->pin, 0)))
    return E_VALUE;

  /* Initialize analog input pin */
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
  interface->event = config->event;
  interface->left = 0;
  interface->unit = config->parent;

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
    case IF_STATUS:
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
  LPC_ADC_Type *reg = interface->unit->parent.reg;

  /* Check buffer alignment */
  assert(!(length & 0x01));

  if (!spinTryLock(&interface->unit->lock))
    return 0;

  reg->CR &= ~CR_SEL_MASK;
  //FIXME Magic
  reg->CR |= CR_START(1) | CR_SEL(interface->channel);

  while (!(reg->GDR & GDR_DONE));

  reg->CR &= ~CR_START_MASK; /* Stop conversion */
  if (reg->GDR & GDR_OVERRUN) /* Return error when overrun occurred */
    return 0;

  uint16_t value = GDR_RESULT_VALUE(reg->GDR, ADC_RESOLUTION);
  memcpy(buffer, &value, sizeof(value));

  spinUnlock(&interface->unit->lock);
  return length;
}
