/*
 * adc_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 15500000
/* Pack and unpack conversion channel and pin function */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)         (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = adcDeinit
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
static struct AdcBase *instance = NULL;
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
  assert(group != NULL);

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
struct AdcBase *adcGetInstance(uint8_t channel __attribute__((unused)))
{
  assert(channel == 0);
  return instance;
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel __attribute__((unused)),
    struct AdcBase *expected, struct AdcBase *interface)
{
  assert(channel == 0);
  return compareExchangePointer(&instance, &expected, interface);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  struct AdcBase * const interface = object;

  assert(config->channel == 0);
  assert(!config->accuracy || config->accuracy == 10 || config->accuracy == 12);

  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  if (!sysPowerStatus(PWR_ADC))
  {
    sysPowerEnable(PWR_ADC);
    sysClockEnable(CLK_ADC);
  }

  interface->channel = 0;
  interface->handler = NULL;
  interface->irq = ADC_IRQ;
  interface->reg = LPC_ADC;

  const bool mode10bit = (config->accuracy == 10);
  const uint32_t fMax = mode10bit ? (MAX_FREQUENCY * 2) : MAX_FREQUENCY;

  assert(config->frequency <= fMax);

  const uint32_t fAdc = config->frequency ? config->frequency : fMax;
  const uint32_t fApb = clockFrequency(MainClock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  assert(divisor <= 256);
  interface->control = CR_CLKDIV(divisor - 1) | (mode10bit ? CR_MODE10BIT : 0);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBase * const interface = object;
  adcSetInstance(interface->channel, interface, NULL);
}
#endif
