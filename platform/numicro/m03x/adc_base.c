/*
 * adc_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/adc_base.h>
#include <halm/platform/numicro/adc_defs.h>
#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 34000000
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
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
        .key = PIN(PORT_B, 0), /* ADC0_CH0 */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 1), /* ADC0_CH1 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_B, 2), /* ADC0_CH2 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_B, 3), /* ADC0_CH3 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_B, 4), /* ADC0_CH4 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 5), /* ADC0_CH5 */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 6), /* ADC0_CH6 */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 7), /* ADC0_CH7 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_B, 8), /* ADC0_CH8 */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_B, 9), /* ADC0_CH9 */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_B, 10), /* ADC0_CH10 */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_B, 11), /* ADC0_CH11 */
        .channel = 0,
        .value = 11
    }, {
        .key = PIN(PORT_B, 12), /* ADC0_CH12 */
        .channel = 0,
        .value = 12
    }, {
        .key = PIN(PORT_B, 13), /* ADC0_CH13 */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_B, 14), /* ADC0_CH14 */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 15), /* ADC0_CH15 */
        .channel = 0,
        .value = 15
    }, {
        .key = PIN(PORT_C, 1), /* ADC0_ST */
        .channel = 0,
        .value = 15
    }, {
        .key = PIN(PORT_C, 13), /* ADC0_ST */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_D, 12), /* ADC0_ST */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_F, 5), /* ADC0_ST */
        .channel = 0,
        .value = 11
    }, {
        .key = PIN(PORT_G, 15), /* ADC0_ST */
        .channel = 0,
        .value = 15
    }, {
        .key = 0 /* End of pin function association list */
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
struct AdcBase *adcGetInstance(uint8_t channel __attribute__((unused)))
{
  assert(channel == 0);
  return instance;
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
  assert(config->channel == 0);
  assert(!config->accuracy || config->accuracy == 12);
  assert(clockFrequency(AdcClock) <= MAX_FREQUENCY);

  struct AdcBase * const interface = object;

  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  interface->reg = NM_ADC;
  interface->irq = ADC_IRQ;
  interface->handler = NULL;
  interface->channel = config->channel;
  interface->control = ADCR_ADEN;

  if (!sysClockStatus(CLK_ADC))
  {
    /* Enable clock to peripheral */
    sysClockEnable(CLK_ADC);
    /* Reset registers to default values */
    sysResetBlock(RST_ADC);

    /* Reset the peripheral */
    sysUnlockReg();
    NM_ADC->ADCR = ADCR_RESET;
    sysLockReg();
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBase * const interface = object;
  adcSetInstance(interface->channel, interface, NULL);
}
#endif
