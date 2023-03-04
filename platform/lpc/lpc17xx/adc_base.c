/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV                   CLK_DIV1
#define MAX_FREQUENCY                 13000000
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
static struct AdcBase *instance = 0;
/*----------------------------------------------------------------------------*/
void ADC_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *interface, PinNumber key)
{
  const struct PinEntry * const entry = pinFind(adcPins, key,
      interface->channel);
  assert(entry);

  /* Initialize pin as input and enable analog pin function */
  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(entry->value));

  return (struct AdcPin){UNPACK_CHANNEL(entry->value)};
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
  assert(config->frequency <= MAX_FREQUENCY);
  assert(!config->accuracy);

  if (!config->shared && !adcSetInstance(config->channel, 0, interface))
    return E_BUSY;

  interface->reg = LPC_ADC;
  interface->irq = ADC_IRQ;
  interface->handler = 0;
  interface->channel = config->channel;

  if (!sysPowerStatus(PWR_ADC))
  {
    sysPowerEnable(PWR_ADC);
    sysClockControl(CLK_ADC, DEFAULT_DIV);
  }

  const uint32_t fAdc = config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t fApb = clockFrequency(MainClock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  assert(divisor <= 256);
  interface->control = CR_PDN | CR_CLKDIV(divisor - 1);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBase * const interface = object;
  adcSetInstance(interface->channel, interface, 0);
}
#endif
