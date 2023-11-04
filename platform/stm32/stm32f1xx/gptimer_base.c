/*
 * gptimer_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/gptimer_base.h>
#include <halm/platform/stm32/gptimer_defs.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct TimerBlockDescriptor
{
  STM_TIM_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Interrupt identifier */
  IrqNumber irq;
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor *findDescriptor(uint8_t);
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor timerBlockEntries[] = {
#ifdef CONFIG_PLATFORM_STM32_TIM1
    {
        .reg = STM_TIM1,
        .clock = CLK_TIM1,
        .reset = RST_TIM1,
        .irq = TIM1_UP_TIM16_IRQ,
        .channel = TIM1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM2
    {
        .reg = STM_TIM2,
        .clock = CLK_TIM2,
        .reset = RST_TIM2,
        .irq = TIM2_IRQ,
        .channel = TIM2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM3
    {
        .reg = STM_TIM3,
        .clock = CLK_TIM3,
        .reset = RST_TIM3,
        .irq = TIM3_IRQ,
        .channel = TIM3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM4
    {
        .reg = STM_TIM4,
        .clock = CLK_TIM4,
        .reset = RST_TIM4,
        .irq = TIM4_IRQ,
        .channel = TIM4
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM5
    {
        .reg = STM_TIM5,
        .clock = CLK_TIM5,
        .reset = RST_TIM5,
        .irq = TIM5_IRQ,
        .channel = TIM5
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM6
    {
        .reg = STM_TIM6,
        .clock = CLK_TIM6,
        .reset = RST_TIM6,
        .irq = DAC_TIM6_IRQ,
        .channel = TIM6
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM7
    {
        .reg = STM_TIM7,
        .clock = CLK_TIM7,
        .reset = RST_TIM7,
        .irq = TIM7_IRQ,
        .channel = TIM7
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM8
    {
        .reg = STM_TIM8,
        .clock = CLK_TIM8,
        .reset = RST_TIM8,
        .irq = TIM8_UP_TIM13_IRQ,
        .channel = TIM8
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM9
    {
        .reg = STM_TIM9,
        .clock = CLK_TIM9,
        .reset = RST_TIM9,
        .irq = IRQ_RESERVED,
        .channel = TIM9
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM10
    {
        .reg = STM_TIM10,
        .clock = CLK_TIM10,
        .reset = RST_TIM10,
        .irq = IRQ_RESERVED,
        .channel = TIM10
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM11
    {
        .reg = STM_TIM11,
        .clock = CLK_TIM11,
        .reset = RST_TIM11,
        .irq = IRQ_RESERVED,
        .channel = TIM11
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM12
    {
        .reg = STM_TIM12,
        .clock = CLK_TIM12,
        .reset = RST_TIM12,
        .irq = TIM8_BRK_TIM12_IRQ,
        .channel = TIM12
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM13
    {
        .reg = STM_TIM13,
        .clock = CLK_TIM13,
        .reset = RST_TIM13,
        .irq = TIM8_UP_TIM13_IRQ,
        .channel = TIM13
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM14
    {
        .reg = STM_TIM14,
        .clock = CLK_TIM14,
        .reset = RST_TIM14,
        .irq = TIM8_TRG_COM_TIM14_IRQ,
        .channel = TIM14
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerPins[] = {
#ifdef CONFIG_PLATFORM_STM32_TIM1
    {
        .key = PIN(PORT_A, 6), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_A, 7), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_A, 8), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = 0
    }, {
        .key = PIN(PORT_A, 9), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = 0
    }, {
        .key = PIN(PORT_A, 10), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = 0
    }, {
        .key = PIN(PORT_A, 11), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = 0
    }, {
        .key = PIN(PORT_A, 12), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = 0
    }, {
        .key = PIN(PORT_B, 0), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_B, 1), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_B, 12), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 0
    }, {
        .key = PIN(PORT_B, 13), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 0
    }, {
        .key = PIN(PORT_B, 14), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 0
    }, {
        .key = PIN(PORT_B, 15), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 7), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 8), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 9), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 10), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 11), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 12), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 13), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 14), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    }, {
        .key = PIN(PORT_E, 15), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = PACK_REMAP(REMAP_TIM1, 3)
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM2
    {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 0
    }, {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 0
    }, {
        .key = PIN(PORT_A, 1), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = 0
    }, {
        .key = PIN(PORT_A, 2), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = 0
    }, {
        .key = PIN(PORT_A, 3), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = 0
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = PACK_REMAP(REMAP_TIM2_LOWER, 2)
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = PACK_REMAP(REMAP_TIM2_LOWER, 2)
    }, {
        .key = PIN(PORT_B, 3), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = PACK_REMAP(REMAP_TIM2_LOWER, 1)
    }, {
        .key = PIN(PORT_B, 10), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = PACK_REMAP(REMAP_TIM2_UPPER, 2)
    }, {
        .key = PIN(PORT_B, 11), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = PACK_REMAP(REMAP_TIM2_UPPER, 2)
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM3
    {
        .key = PIN(PORT_A, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 0
    }, {
        .key = PIN(PORT_A, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 0
    }, {
        .key = PIN(PORT_B, 0), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 0
    }, {
        .key = PIN(PORT_B, 1), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 0
    }, {
        .key = PIN(PORT_B, 4), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = PACK_REMAP(REMAP_TIM3_PARTIAL, 2)
    }, {
        .key = PIN(PORT_B, 5), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = PACK_REMAP(REMAP_TIM3_PARTIAL, 2)
    }, {
        .key = PIN(PORT_C, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = PACK_REMAP(REMAP_TIM3, 3)
    }, {
        .key = PIN(PORT_C, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = PACK_REMAP(REMAP_TIM3, 3)
    }, {
        .key = PIN(PORT_C, 8), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = PACK_REMAP(REMAP_TIM3, 3)
    }, {
        .key = PIN(PORT_C, 9), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = PACK_REMAP(REMAP_TIM3, 3)
    }, {
        .key = PIN(PORT_D, 2), /* TIM3_ETR */
        .channel = CHANNEL_ETR(2),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM4
    {
        .key = PIN(PORT_B, 6), /* TIM4_CH1 */
        .channel = CHANNEL_CH1(3),
        .value = 0
    }, {
        .key = PIN(PORT_B, 7), /* TIM4_CH2 */
        .channel = CHANNEL_CH2(3),
        .value = 0
    }, {
        .key = PIN(PORT_B, 8), /* TIM4_CH3 */
        .channel = CHANNEL_CH3(3),
        .value = 0
    }, {
        .key = PIN(PORT_B, 9), /* TIM4_CH4 */
        .channel = CHANNEL_CH4(3),
        .value = 0
    }, {
        .key = PIN(PORT_D, 12), /* TIM4_CH1 */
        .channel = CHANNEL_CH1(3),
        .value = PACK_REMAP(REMAP_TIM4, 1)
    }, {
        .key = PIN(PORT_D, 13), /* TIM4_CH2 */
        .channel = CHANNEL_CH2(3),
        .value = PACK_REMAP(REMAP_TIM4, 1)
    }, {
        .key = PIN(PORT_D, 14), /* TIM4_CH3 */
        .channel = CHANNEL_CH3(3),
        .value = PACK_REMAP(REMAP_TIM4, 1)
    }, {
        .key = PIN(PORT_D, 15), /* TIM4_CH4 */
        .channel = CHANNEL_CH4(3),
        .value = PACK_REMAP(REMAP_TIM4, 1)
    }, {
        .key = PIN(PORT_E, 0), /* TIM4_ETR */
        .channel = CHANNEL_ETR(3),
        .value = 0
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[14] = {NULL};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor *findDescriptor(uint8_t channel)
{
  for (size_t index = 0; index < ARRAY_SIZE(timerBlockEntries); ++index)
  {
    if (timerBlockEntries[index].channel == channel)
      return &timerBlockEntries[index];
  }

  return NULL;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct GpTimerBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM1
/* Virtual handler */
void TIM1_UP_ISR(void)
{
  if (instances[0] != NULL)
    instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM2
void TIM2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM3
void TIM3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM4
void TIM4_ISR(void)
{
  instances[3]->handler(instances[3]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM5
void TIM5_ISR(void)
{
  instances[4]->handler(instances[4]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM6
/* Virtual handler */
void TIM6_ISR(void)
{
  if (instances[5] != NULL)
    instances[5]->handler(instances[5]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM7
void TIM7_ISR(void)
{
  instances[6]->handler(instances[6]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM8
/* Virtual handler */
void TIM8_UP_ISR(void)
{
  if (instances[7] != NULL)
    instances[7]->handler(instances[7]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM12
/* Virtual handler */
void TIM12_ISR(void)
{
  if (instances[11] != NULL)
    instances[11]->handler(instances[11]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM13
/* Virtual handler */
void TIM13_ISR(void)
{
  if (instances[12] != NULL)
    instances[12]->handler(instances[12]);
}
#endif
/*----------------------------------------------------------------------------*/
/* Virtual handler */
#ifdef CONFIG_PLATFORM_STM32_TIM14
void TIM14_ISR(void)
{
  if (instances[13] != NULL)
    instances[13]->handler(instances[13]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer)
{
  const uint32_t ahbClock = clockFrequency(MainClock);
  uint32_t apbClock = 0;

  if (timer->channel == TIM1
      || (timer->channel >= TIM8 && timer->channel <= TIM11))
  {
    apbClock = clockFrequency(Apb2Clock);
  }
  else
    apbClock = clockFrequency(Apb1Clock);

  return apbClock == ahbClock ? ahbClock : apbClock * 2;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;

  const struct TimerBlockDescriptor * const entry =
      findDescriptor(config->channel);

  assert(entry != NULL);
  if (!setInstance(config->channel, timer))
    return E_BUSY;

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  timer->channel = config->channel;
  timer->handler = NULL;
  timer->irq = entry->irq;
  timer->reg = entry->reg;
  timer->resolution = 16;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;
  const struct TimerBlockDescriptor * const entry =
      findDescriptor(timer->channel);

  sysClockDisable(entry->clock);
  instances[timer->channel] = NULL;
}
#endif
