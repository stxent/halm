/*
 * adc_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 15500000
/* Pack and unpack conversion channel and pin function */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)         (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry adcPins[] = {
    {
        .begin = PIN(0, 11),
        .end = PIN(0, 15),
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .begin = PIN(0, 16),
        .end = PIN(0, 16),
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .begin = PIN(0, 22),
        .end = PIN(0, 23),
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct AdcBase *instance = 0;
/*----------------------------------------------------------------------------*/
void ADC_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *interface, PinNumber key)
{
  const struct PinGroupEntry * const group = pinGroupFind(adcPins, key,
      interface->channel);
  assert(group);

  const unsigned int offset = PIN_TO_OFFSET(key) - PIN_TO_OFFSET(group->begin);
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  /* Enable analog mode */
  pinSetFunction(pin, PIN_ANALOG);
  /* Set analog pin function */
  pinSetFunction(pin, UNPACK_FUNCTION(group->value));

  return (struct AdcPin){UNPACK_CHANNEL(group->value) + offset};
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
void adcResetInstance(uint8_t channel __attribute__((unused)))
{
  instance = 0;
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel __attribute__((unused)),
    struct AdcBase *object)
{
  assert(channel == 0);

  void *expected = 0;
  return compareExchangePointer(&instance, &expected, object);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  struct AdcBase * const interface = object;

  assert(config->channel == 0);
  assert(!config->accuracy || config->accuracy == 10 || config->accuracy == 12);

  interface->reg = LPC_ADC;
  interface->irq = ADC_IRQ;
  interface->handler = 0;
  interface->channel = config->channel;

  if (!sysPowerStatus(PWR_ADC))
  {
    sysPowerEnable(PWR_ADC);
    sysClockEnable(CLK_ADC);
  }

  const bool mode10bit = (config->accuracy == 10);
  const uint32_t fMax = mode10bit ? (MAX_FREQUENCY * 2) : MAX_FREQUENCY;

  assert(config->frequency <= fMax);

  const uint32_t fAdc = config->frequency ? config->frequency : fMax;
  const uint32_t fApb = clockFrequency(MainClock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  interface->control = CR_CLKDIV(divisor - 1) | (mode10bit ? CR_MODE10BIT : 0);
  return E_OK;
}
