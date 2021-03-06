/*
 * adc_base.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 4500000
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

  const uint8_t function = UNPACK_FUNCTION(entry->value);
  const uint8_t index = UNPACK_CHANNEL(entry->value);

  /* Fill pin structure and initialize pin as input */
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  /* Enable analog mode */
  pinSetFunction(pin, PIN_ANALOG);
  /* Set analog pin function */
  pinSetFunction(pin, function);

  return (struct AdcPin){index};
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel __attribute__((unused)),
    struct AdcBase *expected, struct AdcBase *interface)
{
  assert(channel == 0);

  const IrqState state = irqSave();
  bool status = false;

  if (instance == expected)
  {
    instance = interface;
    status = true;
  }

  irqRestore(state);
  return status;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  struct AdcBase * const interface = object;

  assert(config->channel == 0);
  assert(config->frequency <= MAX_FREQUENCY);
  assert(!config->accuracy || (config->accuracy > 2 && config->accuracy < 11));

  if (!config->shared && !adcSetInstance(config->channel, 0, interface))
    return E_BUSY;

  interface->reg = LPC_ADC;
  interface->irq = ADC_IRQ;
  interface->handler = 0;
  interface->channel = config->channel;

  if (!sysPowerStatus(PWR_ADC))
  {
    sysPowerEnable(PWR_ADC);
    sysClockEnable(CLK_ADC);
  }

  const uint32_t ticks = config->accuracy ? (10 - config->accuracy) : 0;
  const uint32_t fAdc = config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t fApb = clockFrequency(MainClock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  interface->control = CR_CLKDIV(divisor - 1) | CR_CLKS(ticks);
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
