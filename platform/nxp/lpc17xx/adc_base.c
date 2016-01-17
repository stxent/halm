/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gen_1/adc_base.h>
#include <platform/nxp/gen_1/adc_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV                     CLK_DIV1
/* Pack or unpack conversion channel and pin function */
#define PACK_VALUE(function, channel)   (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)           (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)          ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct AdcUnitBase *state,
    struct AdcUnitBase *);
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *, const void *);
static void adcUnitDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = 0, /* Abstract class */
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry adcPins[] = {
    {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 23), /* AD0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 24), /* AD1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(0, 25), /* AD2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(0, 26), /* AD3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(1, 30), /* AD4 */
        .channel = 0,
        .value = PACK_VALUE(3, 4)
    }, {
        .key = PIN(1, 31), /* AD5 */
        .channel = 0,
        .value = PACK_VALUE(3, 5)
    }, {
        .key = PIN(0, 3), /* AD6 */
        .channel = 0,
        .value = PACK_VALUE(2, 6)
    }, {
        .key = PIN(0, 2), /* AD7 */
        .channel = 0,
        .value = PACK_VALUE(2, 7)
    }, {
        .key = 0 /* End of pin function association list */
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

  return compareExchangePointer((void **)(descriptors + channel),
      state, unit) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void ADC_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void adcConfigPin(const struct AdcUnitBase *unit, pinNumber key,
    struct AdcPin *adcPin)
{
  const struct PinEntry * const entry = pinFind(adcPins, key, unit->channel);
  assert(entry);

  const uint8_t function = UNPACK_FUNCTION(entry->value);
  const uint8_t index = UNPACK_CHANNEL(entry->value);

  /* Fill pin structure and initialize pin as input */
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  /* Set analog pin function */
  pinSetFunction(pin, function);

  adcPin->channel = index;
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitBaseConfig * const config = configBase;
  struct AdcUnitBase * const unit = object;
  enum result res;

  unit->channel = config->channel;
  unit->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(unit->channel, 0, unit)) != E_OK)
    return res;

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
