/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <memory.h>
#include <platform/nxp/adc_base.h>
#include <platform/nxp/adc_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/* Pack conversion channel and pin function in one number */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct AdcUnitBase *state,
    struct AdcUnitBase *);
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *, const void *);
static void adcUnitDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = sizeof(struct AdcUnitBase),
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry adcPins[] = {
    {
        /* From AD0 to AD3 */
        .begin = PIN(0, 23),
        .end = PIN(0, 26),
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        /* From AD4 to AD5 */
        .begin = PIN(1, 30),
        .end = PIN(1, 31),
        .channel = 0,
        .value = PACK_VALUE(3, 4)
    }, {
        /* AD7 */
        .begin = PIN(0, 2),
        .end = PIN(0, 2),
        .channel = 0,
        .value = PACK_VALUE(2, 7)
    }, {
        /* AD6 */
        .begin = PIN(0, 3),
        .end = PIN(0, 3),
        .channel = 0,
        .value = PACK_VALUE(2, 6)
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcUnitBase = &adcUnitTable;
static struct AdcUnitBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct AdcUnitBase *state, struct AdcUnitBase *unit)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      unit) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void ADC_ISR()
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitBaseConfig * const config = configBase;
  struct AdcUnitBase * const unit = object;
  enum result res;

  /* Try to set peripheral descriptor */
  unit->channel = config->channel;
  if ((res = setDescriptor(unit->channel, 0, unit)) != E_OK)
    return res;

  unit->handler = 0;

  sysPowerEnable(PWR_ADC);
  sysClockControl(CLK_ADC, DEFAULT_DIV);

  unit->irq = ADC_IRQ;
  unit->reg = LPC_ADC;

  LPC_ADC_Type * const reg = unit->reg;

  /* Enable converter and set system clock divider */
  reg->CR = CR_PDN | CR_CLKDIV(clockFrequency(MainClock) / 13000000);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  const struct AdcUnitBase * const unit = object;

  sysPowerDisable(PWR_ADC);
  setDescriptor(unit->channel, unit, 0);
}
