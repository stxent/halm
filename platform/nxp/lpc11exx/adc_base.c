/*
 * adc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/adc_base.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <halm/platform/nxp/lpc11exx/clocking.h>
#include <halm/platform/nxp/lpc11exx/system.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 4500000
/* Pack and unpack conversion channel and pin function */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)         (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t, const struct AdcUnitBase *state,
    struct AdcUnitBase *);
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *);
#else
#define adcUnitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = 0, /* Abstract class */
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
        .key = PIN(0, 12), /* AD1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(0, 13), /* AD2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(0, 14), /* AD3 */
        .channel = 0,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(0, 15), /* AD4 */
        .channel = 0,
        .value = PACK_VALUE(2, 4)
    }, {
        .key = PIN(0, 16), /* AD5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(0, 22), /* AD6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(0, 23), /* AD7 */
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
static bool setDescriptor(uint8_t channel, const struct AdcUnitBase *state,
    struct AdcUnitBase *unit)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state, unit);
}
/*----------------------------------------------------------------------------*/
void ADC_ISR(void)
{
  struct AdcUnitBase * const descriptor = descriptors[0];

  descriptor->handler(descriptor->instance);
}
/*----------------------------------------------------------------------------*/
void adcConfigPin(const struct AdcUnitBase *unit, PinNumber key,
    struct AdcPin *adcPin)
{
  const struct PinEntry * const entry = pinFind(adcPins, key, unit->channel);
  assert(entry);

  const uint8_t function = UNPACK_FUNCTION(entry->value);
  const uint8_t index = UNPACK_CHANNEL(entry->value);

  /* Fill pin structure and initialize pin as input */
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  /* Enable analog pin mode bit */
  pinSetFunction(pin, PIN_ANALOG);
  /* Set analog pin function */
  pinSetFunction(pin, function);

  adcPin->channel = index;
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitBaseConfig * const config = configBase;
  struct AdcUnitBase * const unit = object;

  unit->channel = config->channel;
  unit->handler = 0;
  unit->instance = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(unit->channel, 0, unit))
    return E_BUSY;

  sysPowerEnable(PWR_ADC);
  sysClockEnable(CLK_ADC);

  unit->irq = ADC_IRQ;
  unit->reg = LPC_ADC;

  /* Configure peripheral registers */

  assert(!config->accuracy || (config->accuracy > 2 && config->accuracy < 11));
  assert(config->frequency <= MAX_FREQUENCY);

  const uint32_t clocks = config->accuracy ? 10 - config->accuracy : 0;
  const uint32_t frequency =
      config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t divisor =
      (clockFrequency(MainClock) + (frequency - 1)) / frequency;
  LPC_ADC_Type * const reg = unit->reg;

  reg->INTEN = 0;
  reg->CR = CR_CLKDIV(divisor - 1) | CR_CLKS(clocks);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *object)
{
  const struct AdcUnitBase * const unit = object;

  sysClockDisable(CLK_ADC);
  sysPowerDisable(PWR_ADC);
  setDescriptor(unit->channel, unit, 0);
}
#endif
