/*
 * adc_unit.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <halm/platform/nxp/gen_1/adc_unit.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *, const void *);
static void adcUnitDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = sizeof(struct AdcUnit),
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcUnit = &adcUnitTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcUnit * const unit = object;

  if (unit->callback)
    unit->callback(unit->callbackArgument);
}
/*----------------------------------------------------------------------------*/
enum result adcUnitRegister(struct AdcUnit *unit, void (*callback)(void *),
    void *argument)
{
  if (!spinTryLock(&unit->lock))
    return E_BUSY;

  if (unit->callback)
  {
    spinUnlock(&unit->lock);
    return E_BUSY;
  }

  unit->callback = callback;
  unit->callbackArgument = argument;

  spinUnlock(&unit->lock);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void adcUnitUnregister(struct AdcUnit *unit)
{
  unit->callback = 0;
  unit->callbackArgument = 0;
}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitConfig * const config = configBase;
  assert(config);

  const struct AdcUnitBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel
  };
  struct AdcUnit * const unit = object;
  enum result res;

  /* Call base class constructor */
  if ((res = AdcUnitBase->init(object, &baseConfig)) != E_OK)
    return res;

  unit->callback = 0;
  unit->callbackArgument = 0;
  unit->lock = SPIN_UNLOCKED;
  unit->base.handler = interruptHandler;

  LPC_ADC_Type * const reg = unit->base.reg;

  /* Disable interrupts for conversion completion */
  reg->INTEN = 0;

  /* Configure priority */
  irqSetPriority(unit->base.irq, config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  AdcUnitBase->deinit(object);
}
