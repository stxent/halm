/*
 * adc_unit.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <halm/platform/nxp/gen_1/adc_unit.h>
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *);
#else
#define adcUnitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = sizeof(struct AdcUnit),
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcUnit = &adcUnitTable;
/*----------------------------------------------------------------------------*/
enum Result adcUnitRegister(struct AdcUnit *unit, void (*handler)(void *),
    void *instance)
{
  assert(instance);

  if (compareExchangePointer((void **)&unit->base.instance, 0, instance))
  {
    unit->base.handler = handler;
    return E_OK;
  }
  else
    return E_BUSY;
}
/*----------------------------------------------------------------------------*/
void adcUnitUnregister(struct AdcUnit *unit)
{
  unit->base.handler = 0;
  unit->base.instance = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitConfig * const config = configBase;
  assert(config);

  const struct AdcUnitBaseConfig baseConfig = {
      .frequency = config->frequency,
      .accuracy = config->accuracy,
      .channel = config->channel
  };
  struct AdcUnit * const unit = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = AdcUnitBase->init(object, &baseConfig)) != E_OK)
    return res;

  LPC_ADC_Type * const reg = unit->base.reg;

  /* Disable interrupts for conversion completion */
  reg->INTEN = 0;

  /* Configure priority */
  irqSetPriority(unit->base.irq, config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *object)
{
  AdcUnitBase->deinit(object);
}
#endif
