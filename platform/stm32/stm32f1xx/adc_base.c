/*
 * adc_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/adc_base.h>
#include <halm/platform/stm32/bdma_circular.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/gen_1/adc_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 14000000

struct AdcBlockDescriptor
{
  STM_ADC_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void adcBaseHandler0(void);
[[gnu::weak]] void adcBaseHandler1(void);
[[gnu::weak]] void adcBaseHandler2(void);

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
static const struct AdcBlockDescriptor adcBlockEntries[] = {
#ifdef CONFIG_PLATFORM_STM32_ADC1
    {
        .reg = STM_ADC1,
        .clock = CLK_ADC1,
        .reset = RST_ADC1,
        .irq = ADC1_2_IRQ,
        .channel = ADC1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_ADC2
    {
        .reg = STM_ADC2,
        .clock = CLK_ADC2,
        .reset = RST_ADC2,
        .irq = ADC1_2_IRQ,
        .channel = ADC2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_ADC3
    {
        .reg = STM_ADC3,
        .clock = CLK_ADC3,
        .reset = RST_ADC3,
        .irq = ADC3_IRQ,
        .channel = ADC3
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry adcPinGroups[] = {
#ifdef CONFIG_PLATFORM_STM32_ADC1
    {
        /* ADC123_IN0 .. ADC123_IN3 and ADC12_IN4 .. ADC12_IN7 for ADC1 */
        .begin = PIN(PORT_A, 0),
        .end = PIN(PORT_A, 7),
        .channel = 0,
        .value = 0
    }, {
        /* ADC12_IN8 .. ADC12_IN9 for ADC1 */
        .begin = PIN(PORT_B, 0),
        .end = PIN(PORT_B, 1),
        .channel = 0,
        .value = 8
    }, {
        /* ADC123_IN10 .. ADC123_IN13 and ADC12_IN14 .. ADC12_IN15 for ADC1 */
        .begin = PIN(PORT_C, 0),
        .end = PIN(PORT_C, 5),
        .channel = 0,
        .value = 10
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_ADC2
    {
        /* ADC123_IN0 .. ADC123_IN3 and ADC12_IN4 .. ADC12_IN7 for ADC2 */
        .begin = PIN(PORT_A, 0),
        .end = PIN(PORT_A, 7),
        .channel = 1,
        .value = 0
    }, {
        /* ADC12_IN8 .. ADC12_IN9 for ADC2 */
        .begin = PIN(PORT_B, 0),
        .end = PIN(PORT_B, 1),
        .channel = 1,
        .value = 8
    }, {
        /* ADC123_IN10 .. ADC123_IN13 and ADC12_IN14 .. ADC12_IN15 for ADC2 */
        .begin = PIN(PORT_C, 0),
        .end = PIN(PORT_C, 5),
        .channel = 1,
        .value = 10
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_ADC3
    {
        /* ADC123_IN0 .. ADC123_IN3 for ADC3 */
        .begin = PIN(PORT_A, 0),
        .end = PIN(PORT_A, 3),
        .channel = 2,
        .value = 0
    }, {
        /* ADC123_IN10 .. ADC123_IN13 for ADC3 */
        .begin = PIN(PORT_C, 0),
        .end = PIN(PORT_C, 3),
        .channel = 2,
        .value = 10
    }, {
        /* ADC3_IN4 .. ADC3_IN8 for ADC3 */
        .begin = PIN(PORT_F, 6),
        .end = PIN(PORT_F, 10),
        .channel = 2,
        .value = 4
    },
#endif
    {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct AdcBase *instances[3] = {NULL};
/*----------------------------------------------------------------------------*/
static const struct AdcBlockDescriptor *findDescriptor(uint8_t channel)
{
  for (size_t index = 0; index < ARRAY_SIZE(adcBlockEntries); ++index)
  {
    if (adcBlockEntries[index].channel == channel)
      return &adcBlockEntries[index];
  }

  return NULL;
}
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_PLATFORM_STM32_ADC1) || defined(CONFIG_PLATFORM_STM32_ADC2)
void ADC1_2_ISR(void)
{
#  ifdef CONFIG_PLATFORM_STM32_ADC1
  if (STM_ADC1->SR & SR_MASK)
    adcBaseHandler0();
#  endif /* CONFIG_PLATFORM_STM32_ADC1 */

#  ifdef CONFIG_PLATFORM_STM32_ADC2
  if (STM_ADC2->SR & SR_MASK)
    adcBaseHandler1();
#  endif /* CONFIG_PLATFORM_STM32_ADC2 */
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC3
void ADC3_ISR(void)
{
  adcBaseHandler2();
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC1
void adcBaseHandler0(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC2
void adcBaseHandler1(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_ADC3
void adcBaseHandler2(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
void *adcMakeCircularDma([[maybe_unused]] uint8_t channel, uint8_t stream,
    enum DmaPriority priority, bool silent)
{
  const struct BdmaCircularConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = DMA_TYPE_P2M,
      .stream = stream,
      .silent = silent
  };

  /* DMA is not supported on ADC2 */
  assert(channel != 1);

  return init(BdmaCircular, &config);
}
/*----------------------------------------------------------------------------*/
struct AdcBase *adcGetInstance(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(instances));
  return instances[channel];
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel, struct AdcBase *expected,
    struct AdcBase *interface)
{
  assert(channel < ARRAY_SIZE(instances));
  return compareExchangePointer(&instances[channel], &expected, interface);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  /* ADC1/2 and ADC3 have different triggers for injected group */
  assert((config->channel <= 1 && (config->injected < 16
          || config->injected == ADC_INJ_SOFTWARE))
      || (config->channel == 2 && config->injected >= 16
          && config->injected < ADC_INJ_EVENT_END));
  /* ADC1/2 and ADC3 have different triggers for regular group */
  assert((config->channel <= 1 && (config->regular < 16
          || config->regular == ADC_SOFTWARE))
      || (config->channel == 2 && config->regular >= 16
          && config->regular < ADC_EVENT_END));
  assert(clockFrequency(AdcClock) <= MAX_FREQUENCY);

  struct AdcBase * const interface = object;
  const struct AdcBlockDescriptor * const entry =
      findDescriptor(config->channel);

  assert(entry != NULL);
  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  if (!sysClockStatus(entry->clock))
  {
    sysClockEnable(entry->clock);
    sysResetPulse(entry->reset);
  }

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->reg = entry->reg;

  if (!irqStatus(entry->irq))
  {
    /* Enable IRQ in NVIC */
    irqSetPriority(entry->irq, CONFIG_PLATFORM_STM32_ADC_PRIORITY);
    irqEnable(entry->irq);
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
