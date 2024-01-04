/*
 * adc_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/adc_base.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/dma_circular.h>
#include <halm/platform/stm32/gen_1/adc_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY_HV 30000000
#define MAX_FREQUENCY_LV 15000000
/*----------------------------------------------------------------------------*/
void adcBaseHandler0(void) __attribute__((weak));
void adcBaseHandler1(void) __attribute__((weak));
void adcBaseHandler2(void) __attribute__((weak));

static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_ADC_NO_DEINIT
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
extern enum VoltageRange volRange;

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
    },{
        /* ADC123_IN10 .. ADC123_IN13 for ADC3 */
        .begin = PIN(PORT_C, 0),
        .end = PIN(PORT_C, 3),
        .channel = 2,
        .value = 10
    }, {
        /* ADC3_IN9 for ADC3 */
        .begin = PIN(PORT_F, 3),
        .end = PIN(PORT_F, 3),
        .channel = 2,
        .value = 9
    }, {
        /* ADC3_IN4 .. ADC3_IN8 for ADC3 */
        .begin = PIN(PORT_F, 6),
        .end = PIN(PORT_F, 10),
        .channel = 2,
        .value = 4
    }, {
        /* ADC3_IN14 .. ADC3_IN15 for ADC3 */
        .begin = PIN(PORT_F, 4),
        .end = PIN(PORT_F, 5),
        .channel = 2,
        .value = 14
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
void ADC_ISR(void)
{
  const uint32_t csr = STM_ADC_COMMON->CSR;

#ifdef CONFIG_PLATFORM_STM32_ADC1
  if (csr & CSR_MASK(0))
    adcBaseHandler0();
#endif

#ifdef CONFIG_PLATFORM_STM32_ADC2
  if (csr & CSR_MASK(1))
    adcBaseHandler1();
#endif

#ifdef CONFIG_PLATFORM_STM32_ADC3
  if (csr & CSR_MASK(2))
    adcBaseHandler2();
#endif
}
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
void *adcMakeCircularDma(uint8_t channel, uint8_t stream,
    enum DmaPriority priority, bool silent)
{
  const struct DmaCircularConfig config = {
      .event = dmaGetEventAdc(channel),
      .priority = priority,
      .type = DMA_TYPE_P2M,
      .stream = stream,
      .silent = silent
  };

  return init(DmaCircular, &config);
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
  assert(config->injected < ADC_INJ_EVENT_END);
  assert(config->regular < ADC_EVENT_END);

  struct AdcBase * const interface = object;
  bool enabled = false;

  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->reg = NULL;

  switch (config->channel)
  {
#ifdef CONFIG_PLATFORM_STM32_ADC1
    case 0:
      if (!(enabled = sysClockStatus(CLK_ADC1)))
        sysClockEnable(CLK_ADC1);
      interface->reg = STM_ADC1;
      break;
#endif

#ifdef CONFIG_PLATFORM_STM32_ADC2
    case 1:
      if (!(enabled = sysClockStatus(CLK_ADC2)))
        sysClockEnable(CLK_ADC2);
      interface->reg = STM_ADC2;
      break;
#endif

#ifdef CONFIG_PLATFORM_STM32_ADC3
    case 2:
      if (!(enabled = sysClockStatus(CLK_ADC3)))
        sysClockEnable(CLK_ADC3);
      interface->reg = STM_ADC3;
      break;
#endif

    default:
      break;
  }
  assert(interface->reg != NULL);

  if (!enabled)
  {
    STM_ADC_Type * const reg = interface->reg;

    /* Reset current peripheral manually, APB reset is global to all ADC */
    reg->CR2 = 0;
    reg->CR1 = 0;
    reg->SR = 0;
  }

  if (!irqStatus(ADC_IRQ))
  {
    /* Initial setup for all three peripherals */

    /* Reconfigure ADDCLK, clock is global to all ADC peripherals */
    const uint32_t clock = clockFrequency(Apb2Clock);
    uint32_t prescaler;

    if (volRange > VR_2V1_2V4)
      prescaler = (clock + MAX_FREQUENCY_HV - 1) / MAX_FREQUENCY_HV;
    else
      prescaler = (clock + MAX_FREQUENCY_LV - 1) / MAX_FREQUENCY_LV;
    assert(prescaler <= 8);

    if (prescaler < 2)
      prescaler = 2;
    if (prescaler & 1)
      ++prescaler;

    STM_ADC_COMMON->CCR = CCR_ADCPRE((prescaler >> 1) - 1);

    /* Enable IRQ in NVIC, interrupt vector is global to all ADC peripherals */
    irqSetPriority(ADC_IRQ, CONFIG_PLATFORM_STM32_ADC_PRIORITY);
    irqEnable(ADC_IRQ);
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
