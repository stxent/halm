/*
 * adc_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <memory.h>
#include <platform/nxp/adc_base.h>
#include <platform/nxp/adc_defs.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
/* Pack conversion channel and pin function in one number */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct AdcUnitBase *);
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
const struct PinEntry adcPins[] = {
    {
        .key = PIN(0, 11), /* AD0 */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 0), /* AD1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(1, 1), /* AD2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(1, 2), /* AD3 */
        .channel = 0,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(1, 3), /* AD4 */
        .channel = 0,
        .value = PACK_VALUE(2, 4)
    }, {
        .key = PIN(1, 4), /* AD5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(1, 10), /* AD6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(1, 11), /* AD7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcUnitBase = &adcUnitTable;
static struct AdcUnitBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct AdcUnitBase *unit)
{
  assert(channel < sizeof(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), 0,
      unit) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void ADC_ISR()
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configPtr)
{
  const struct AdcUnitBaseConfig * const config = configPtr;
  struct AdcUnitBase * const unit = object;
  enum result res;

  /* Try to set peripheral descriptor */
  unit->channel = config->channel;
  if ((res = setDescriptor(unit->channel, unit)) != E_OK)
    return res;

  unit->handler = 0;

  sysPowerEnable(PWR_ADC);
  sysClockEnable(CLK_ADC);

  unit->irq = ADC_IRQ;
  unit->reg = LPC_ADC;

  LPC_ADC_Type * const reg = unit->reg;

  /* Set system clock divider */
  reg->CR = CR_CLKDIV(clockFrequency(MainClock) / 4500000);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  struct AdcUnitBase * const unit = object;

  sysClockDisable(CLK_ADC);
  sysPowerDisable(PWR_ADC);
  setDescriptor(unit->channel, 0);
}
