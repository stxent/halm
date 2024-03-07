/*
 * adc_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/adc_base.h>
#include <halm/platform/stm32/bdma_circular.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/gen_2/adc_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 14000000
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void adcBaseHandler0(void);

static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
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
const struct PinGroupEntry adcPinGroups[] = {
    {
        /* ADC_IN0 .. ADC_IN7 */
        .begin = PIN(PORT_A, 0),
        .end = PIN(PORT_A, 7),
        .channel = 0,
        .value = 0
    }, {
        /* ADC_IN8 .. ADC_IN9 */
        .begin = PIN(PORT_B, 0),
        .end = PIN(PORT_B, 1),
        .channel = 0,
        .value = 8
    }, {
        /* ADC_IN10 .. ADC_IN15 */
        .begin = PIN(PORT_C, 0),
        .end = PIN(PORT_C, 5),
        .channel = 0,
        .value = 10
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct AdcBase *instance = NULL;
/*----------------------------------------------------------------------------*/
void ADC1_ISR(void)
{
  if (STM_ADC->ISR & ISR_MASK)
    adcBaseHandler0();
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC1
void adcBaseHandler0(void)
{
  instance->handler(instance);
}
#endif
/*----------------------------------------------------------------------------*/
void *adcMakeCircularDma(uint8_t, uint8_t stream, enum DmaPriority priority,
    bool silent)
{
  const struct BdmaCircularConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = DMA_TYPE_P2M,
      .stream = stream,
      .silent = silent
  };

  return init(BdmaCircular, &config);
}
/*----------------------------------------------------------------------------*/
struct AdcBase *adcGetInstance([[maybe_unused]] uint8_t channel)
{
  assert(channel == 0);
  return instance;
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance([[maybe_unused]] uint8_t channel,
    struct AdcBase *expected, struct AdcBase *interface)
{
  assert(channel == 0);
  return compareExchangePointer(&instance, &expected, interface);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  assert(config->injected < ADC_INJ_EVENT_END);
  assert(config->regular < ADC_EVENT_END);
  assert(clockFrequency(AdcClock) <= MAX_FREQUENCY);

  struct AdcBase * const interface = object;

  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  if (!sysClockStatus(CLK_ADC))
  {
    sysClockEnable(CLK_ADC);
    sysResetEnable(RST_ADC);
    sysResetDisable(RST_ADC);
  }

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->reg = STM_ADC;

  if (!irqStatus(ADC1_COMP_IRQ))
  {
    /* Enable IRQ in NVIC */
    irqSetPriority(ADC1_COMP_IRQ, CONFIG_PLATFORM_STM32_ADC_PRIORITY);
    irqEnable(ADC1_COMP_IRQ);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBase * const interface = object;
  adcSetInstance(interface->channel, interface, NULL);
}
#endif
