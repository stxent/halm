/*
 * gptimer_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/gptimer_base.h>
#include <halm/platform/stm32/gptimer_defs.h>
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
  /* Peripheral channel number */
  uint8_t channel;
  /* Peripheral channel flags */
  uint8_t flags;
};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor *findDescriptor(uint8_t);
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
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
        .irq = TIM1_UP_TIM10_IRQ,
        .channel = TIM1,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN | TIMER_FLAG_CONTROL
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM2
    {
        .reg = STM_TIM2,
        .clock = CLK_TIM2,
        .reset = RST_TIM2,
        .irq = TIM2_IRQ,
        .channel = TIM2,
        .flags = TIMER_FLAG_32_BIT | TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM3
    {
        .reg = STM_TIM3,
        .clock = CLK_TIM3,
        .reset = RST_TIM3,
        .irq = TIM3_IRQ,
        .channel = TIM3,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM4
    {
        .reg = STM_TIM4,
        .clock = CLK_TIM4,
        .reset = RST_TIM4,
        .irq = TIM4_IRQ,
        .channel = TIM4,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM5
    {
        .reg = STM_TIM5,
        .clock = CLK_TIM5,
        .reset = RST_TIM5,
        .irq = TIM5_IRQ,
        .channel = TIM5,
        .flags = TIMER_FLAG_32_BIT | TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM6
    {
        .reg = STM_TIM6,
        .clock = CLK_TIM6,
        .reset = RST_TIM6,
        .irq = TIM6_DAC_IRQ,
        .channel = TIM6,
        .flags = TIMER_FLAG_DMA
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM7
    {
        .reg = STM_TIM7,
        .clock = CLK_TIM7,
        .reset = RST_TIM7,
        .irq = TIM7_IRQ,
        .channel = TIM7,
        .flags = TIMER_FLAG_DMA
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM8
    {
        .reg = STM_TIM8,
        .clock = CLK_TIM8,
        .reset = RST_TIM8,
        .irq = TIM8_UP_TIM13_IRQ,
        .channel = TIM8,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_UPDOWN | TIMER_FLAG_CONTROL
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM9
    {
        .reg = STM_TIM9,
        .clock = CLK_TIM9,
        .reset = RST_TIM9,
        .irq = TIM1_BRK_TIM9_IRQ,
        .channel = TIM9,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM10
    {
        .reg = STM_TIM10,
        .clock = CLK_TIM10,
        .reset = RST_TIM10,
        .irq = TIM1_UP_TIM10_IRQ,
        .channel = TIM10,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM11
    {
        .reg = STM_TIM11,
        .clock = CLK_TIM11,
        .reset = RST_TIM11,
        .irq = TIM1_TRG_COM_TIM11_IRQ,
        .channel = TIM11,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM12
    {
        .reg = STM_TIM12,
        .clock = CLK_TIM12,
        .reset = RST_TIM12,
        .irq = TIM8_BRK_TIM12_IRQ,
        .channel = TIM12,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM13
    {
        .reg = STM_TIM13,
        .clock = CLK_TIM13,
        .reset = RST_TIM13,
        .irq = TIM8_UP_TIM13_IRQ,
        .channel = TIM13,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM14
    {
        .reg = STM_TIM14,
        .clock = CLK_TIM14,
        .reset = RST_TIM14,
        .irq = TIM8_TRG_COM_TIM14_IRQ,
        .channel = TIM14,
        .flags = 0
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerPins[] = {
#ifdef CONFIG_PLATFORM_STM32_TIM1
    {
        .key = PIN(PORT_A, 6), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 7), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 8), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 9), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 10), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 11), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 12), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 0), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 1), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 12), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 13), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 14), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 15), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 7), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 8), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 9), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 10), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 11), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 12), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 13), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 14), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = 1
    }, {
        .key = PIN(PORT_E, 15), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM2
    {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 1), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 2), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 3), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 1
    }, {
        .key = PIN(PORT_B, 3), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = 1
    }, {
        .key = PIN(PORT_B, 10), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = 1
    }, {
        .key = PIN(PORT_B, 11), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = 1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM3
    {
        .key = PIN(PORT_A, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 2
    }, {
        .key = PIN(PORT_A, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 2
    }, {
        .key = PIN(PORT_B, 0), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 2
    }, {
        .key = PIN(PORT_B, 1), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 2
    }, {
        .key = PIN(PORT_B, 4), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 2
    }, {
        .key = PIN(PORT_B, 5), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 2
    }, {
        .key = PIN(PORT_C, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 2
    }, {
        .key = PIN(PORT_C, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 2
    }, {
        .key = PIN(PORT_C, 8), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 2
    }, {
        .key = PIN(PORT_C, 9), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 2
    }, {
        .key = PIN(PORT_D, 2), /* TIM3_ETR */
        .channel = CHANNEL_ETR(2),
        .value = 2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM4
    {
        .key = PIN(PORT_B, 6), /* TIM4_CH1 */
        .channel = CHANNEL_CH1(3),
        .value = 2
    }, {
        .key = PIN(PORT_B, 7), /* TIM4_CH2 */
        .channel = CHANNEL_CH2(3),
        .value = 2
    }, {
        .key = PIN(PORT_B, 8), /* TIM4_CH3 */
        .channel = CHANNEL_CH3(3),
        .value = 2
    }, {
        .key = PIN(PORT_B, 9), /* TIM4_CH4 */
        .channel = CHANNEL_CH4(3),
        .value = 2
    }, {
        .key = PIN(PORT_D, 12), /* TIM4_CH1 */
        .channel = CHANNEL_CH1(3),
        .value = 2
    }, {
        .key = PIN(PORT_D, 13), /* TIM4_CH2 */
        .channel = CHANNEL_CH2(3),
        .value = 2
    }, {
        .key = PIN(PORT_D, 14), /* TIM4_CH3 */
        .channel = CHANNEL_CH3(3),
        .value = 2
    }, {
        .key = PIN(PORT_D, 15), /* TIM4_CH4 */
        .channel = CHANNEL_CH4(3),
        .value = 2
    }, {
        .key = PIN(PORT_E, 0), /* TIM4_ETR */
        .channel = CHANNEL_ETR(3),
        .value = 2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM5
    {
        .key = PIN(PORT_A, 0), /* TIM5_CH1 */
        .channel = CHANNEL_CH1(4),
        .value = 2
    }, {
        .key = PIN(PORT_A, 1), /* TIM5_CH2 */
        .channel = CHANNEL_CH2(4),
        .value = 2
    }, {
        .key = PIN(PORT_A, 2), /* TIM5_CH3 */
        .channel = CHANNEL_CH3(4),
        .value = 2
    }, {
        .key = PIN(PORT_A, 3), /* TIM5_CH4 */
        .channel = CHANNEL_CH4(4),
        .value = 2
    }, {
        .key = PIN(PORT_H, 10), /* TIM5_CH1 */
        .channel = CHANNEL_CH1(4),
        .value = 2
    }, {
        .key = PIN(PORT_H, 11), /* TIM5_CH2 */
        .channel = CHANNEL_CH2(4),
        .value = 2
    }, {
        .key = PIN(PORT_H, 12), /* TIM5_CH3 */
        .channel = CHANNEL_CH3(4),
        .value = 2
    }, {
        .key = PIN(PORT_I, 0), /* TIM5_CH4 */
        .channel = CHANNEL_CH4(4),
        .value = 2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM8
    {
        .key = PIN(PORT_A, 0), /* TIM8_ETR */
        .channel = CHANNEL_ETR(7),
        .value = 3
    }, {
        .key = PIN(PORT_A, 5), /* TIM8_CH1N */
        .channel = CHANNEL_CH1N(7),
        .value = 3
    }, {
        .key = PIN(PORT_A, 6), /* TIM8_BKIN */
        .channel = CHANNEL_BKIN(7),
        .value = 3
    }, {
        .key = PIN(PORT_A, 7), /* TIM8_CH1N */
        .channel = CHANNEL_CH1N(7),
        .value = 3
    }, {
        .key = PIN(PORT_B, 0), /* TIM8_CH2N */
        .channel = CHANNEL_CH2N(7),
        .value = 3
    }, {
        .key = PIN(PORT_B, 1), /* TIM8_CH3N */
        .channel = CHANNEL_CH3N(7),
        .value = 3
    }, {
        .key = PIN(PORT_B, 14), /* TIM8_CH2N */
        .channel = CHANNEL_CH2N(7),
        .value = 3
    }, {
        .key = PIN(PORT_B, 15), /* TIM8_CH3N */
        .channel = CHANNEL_CH3N(7),
        .value = 3
    }, {
        .key = PIN(PORT_C, 6), /* TIM8_CH1 */
        .channel = CHANNEL_CH1(7),
        .value = 3
    }, {
        .key = PIN(PORT_C, 7), /* TIM8_CH2 */
        .channel = CHANNEL_CH2(7),
        .value = 3
    }, {
        .key = PIN(PORT_C, 8), /* TIM8_CH3 */
        .channel = CHANNEL_CH3(7),
        .value = 3
    }, {
        .key = PIN(PORT_C, 9), /* TIM8_CH4 */
        .channel = CHANNEL_CH4(7),
        .value = 3
    }, {
        .key = PIN(PORT_H, 13), /* TIM8_CH1N */
        .channel = CHANNEL_CH1N(7),
        .value = 3
    }, {
        .key = PIN(PORT_H, 14), /* TIM8_CH2N */
        .channel = CHANNEL_CH2N(7),
        .value = 3
    }, {
        .key = PIN(PORT_H, 15), /* TIM8_CH3N */
        .channel = CHANNEL_CH3N(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 2), /* TIM8_CH4 */
        .channel = CHANNEL_CH4(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 3), /* TIM8_ETR */
        .channel = CHANNEL_ETR(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 4), /* TIM8_BKIN */
        .channel = CHANNEL_BKIN(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 5), /* TIM8_CH1 */
        .channel = CHANNEL_CH1(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 6), /* TIM8_CH2 */
        .channel = CHANNEL_CH2(7),
        .value = 3
    }, {
        .key = PIN(PORT_I, 7), /* TIM8_CH3 */
        .channel = CHANNEL_CH3(7),
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM9
    {
        .key = PIN(PORT_A, 2), /* TIM9_CH1 */
        .channel = CHANNEL_CH1(8),
        .value = 3
    }, {
        .key = PIN(PORT_A, 3), /* TIM9_CH2 */
        .channel = CHANNEL_CH2(8),
        .value = 3
    }, {
        .key = PIN(PORT_E, 5), /* TIM9_CH1 */
        .channel = CHANNEL_CH1(8),
        .value = 3
    }, {
        .key = PIN(PORT_E, 6), /* TIM9_CH2 */
        .channel = CHANNEL_CH2(8),
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM10
    {
        .key = PIN(PORT_B, 8), /* TIM10_CH1 */
        .channel = CHANNEL_CH1(9),
        .value = 3
    }, {
        .key = PIN(PORT_F, 6), /* TIM10_CH1 */
        .channel = CHANNEL_CH1(9),
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM11
    {
        .key = PIN(PORT_B, 9), /* TIM11_CH1 */
        .channel = CHANNEL_CH1(10),
        .value = 3
    }, {
        .key = PIN(PORT_F, 7), /* TIM11_CH1 */
        .channel = CHANNEL_CH1(10),
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM12
    {
        .key = PIN(PORT_B, 14), /* TIM12_CH1 */
        .channel = CHANNEL_CH1(11),
        .value = 9
    }, {
        .key = PIN(PORT_B, 15), /* TIM12_CH2 */
        .channel = CHANNEL_CH2(11),
        .value = 9
    }, {
        .key = PIN(PORT_H, 6), /* TIM12_CH1 */
        .channel = CHANNEL_CH1(11),
        .value = 9
    }, {
        .key = PIN(PORT_H, 9), /* TIM12_CH2 */
        .channel = CHANNEL_CH2(11),
        .value = 9
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM13
    {
        .key = PIN(PORT_A, 6), /* TIM13_CH1 */
        .channel = CHANNEL_CH1(12),
        .value = 9
    }, {
        .key = PIN(PORT_F, 8), /* TIM13_CH1 */
        .channel = CHANNEL_CH1(12),
        .value = 9
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM14
    {
        .key = PIN(PORT_A, 7), /* TIM14_CH1 */
        .channel = CHANNEL_CH1(13),
        .value = 9
    }, {
        .key = PIN(PORT_F, 9), /* TIM14_CH1 */
        .channel = CHANNEL_CH1(13),
        .value = 9
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
#ifdef CONFIG_PLATFORM_STM32_TIM9
/* Virtual handler */
void TIM9_ISR(void)
{
  if (instances[8] != NULL)
    instances[8]->handler(instances[8]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM10
/* Virtual handler */
void TIM10_ISR(void)
{
  if (instances[9] != NULL)
    instances[9]->handler(instances[9]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM11
/* Virtual handler */
void TIM11_ISR(void)
{
  if (instances[10] != NULL)
    instances[10]->handler(instances[10]);
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
  sysResetPulse(entry->reset);

  timer->channel = config->channel;
  timer->flags = entry->flags;
  timer->handler = NULL;
  timer->irq = entry->irq;
  timer->reg = entry->reg;

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
