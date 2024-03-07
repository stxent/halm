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
[[gnu::weak]] void adcBaseHandler0(void);

static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
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
static struct AdcBase *instance = NULL;
/*----------------------------------------------------------------------------*/
void ADC_ISR(void)
{
  adcBaseHandler0();
}
/*----------------------------------------------------------------------------*/
void adcBaseHandler0(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *interface, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(adcPins, key,
      interface->channel);
  assert(pinEntry != NULL);

  const uint8_t function = UNPACK_FUNCTION(pinEntry->value);
  const uint8_t index = UNPACK_CHANNEL(pinEntry->value);

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
struct AdcBase *adcGetInstance([[maybe_unused]] uint8_t channel)
{
  assert(channel == 0);
  return instance;
}
/*----------------------------------------------------------------------------*/
void adcReleasePin([[maybe_unused]] struct AdcPin adcPin)
{
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance([[maybe_unused]] uint8_t channel, struct AdcBase *expected,
    struct AdcBase *interface)
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
  assert(config->channel == 0);
  assert(config->frequency <= MAX_FREQUENCY);
  assert(!config->accuracy || (config->accuracy > 2 && config->accuracy < 11));

  struct AdcBase * const interface = object;

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

  const uint32_t ticks = config->accuracy ? (10 - config->accuracy) : 0;
  const uint32_t fAdc = config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t fApb = clockFrequency(MainClock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  assert(divisor <= 256);
  interface->control = CR_CLKDIV(divisor - 1) | CR_CLKS(ticks);

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
