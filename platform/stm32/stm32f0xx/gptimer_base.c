/*
 * gptimer_base.c
 * Copyright (C) 2020 xent
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
        .irq = TIM1_BRK_UP_TRG_COM_IRQ,
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
#ifdef CONFIG_PLATFORM_STM32_TIM14
    {
        .reg = STM_TIM14,
        .clock = CLK_TIM14,
        .reset = RST_TIM14,
        .irq = TIM14_IRQ,
        .channel = TIM14,
        .flags = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM15
    {
        .reg = STM_TIM15,
        .clock = CLK_TIM15,
        .reset = RST_TIM15,
        .irq = TIM15_IRQ,
        .channel = TIM15,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_CONTROL
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM16
    {
        .reg = STM_TIM16,
        .clock = CLK_TIM16,
        .reset = RST_TIM16,
        .irq = TIM16_IRQ,
        .channel = TIM16,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_CONTROL
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM17
    {
        .reg = STM_TIM17,
        .clock = CLK_TIM17,
        .reset = RST_TIM17,
        .irq = TIM17_IRQ,
        .channel = TIM17,
        .flags = TIMER_FLAG_DMA | TIMER_FLAG_CONTROL
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerPins[] = {
#ifdef CONFIG_PLATFORM_STM32_TIM1
    {
        .key = PIN(PORT_A, 6), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 7), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 8), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 9), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 10), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 11), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = 2
    }, {
        .key = PIN(PORT_A, 12), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 0), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 1), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 12), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 13), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 14), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 2
    }, {
        .key = PIN(PORT_B, 15), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 2
    }, {
        .key = PIN(PORT_E, 7), /* TIM1_ETR */
        .channel = CHANNEL_ETR(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 8), /* TIM1_CH1N */
        .channel = CHANNEL_CH1N(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 9), /* TIM1_CH1 */
        .channel = CHANNEL_CH1(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 10), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 11), /* TIM1_CH2 */
        .channel = CHANNEL_CH2(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 12), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 13), /* TIM1_CH3 */
        .channel = CHANNEL_CH3(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 14), /* TIM1_CH4 */
        .channel = CHANNEL_CH4(0),
        .value = 0
    }, {
        .key = PIN(PORT_E, 15), /* TIM1_BKIN */
        .channel = CHANNEL_BKIN(0),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM2
    {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 0), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 1), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 2), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 3), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 5), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 5), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_CH1(1),
        .value = 2
    }, {
        .key = PIN(PORT_A, 15), /* TIM2_CH1_ETR */
        .channel = CHANNEL_ETR(1),
        .value = 2
    }, {
        .key = PIN(PORT_B, 3), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = 2
    }, {
        .key = PIN(PORT_B, 10), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = 2
    }, {
        .key = PIN(PORT_B, 11), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = 2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM3
    {
        .key = PIN(PORT_A, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 1
    }, {
        .key = PIN(PORT_A, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 1
    }, {
        .key = PIN(PORT_B, 0), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 1
    }, {
        .key = PIN(PORT_B, 1), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 1
    }, {
        .key = PIN(PORT_B, 4), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 1
    }, {
        .key = PIN(PORT_B, 5), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 1
    }, {
        .key = PIN(PORT_C, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 0
    }, {
        .key = PIN(PORT_C, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 0
    }, {
        .key = PIN(PORT_C, 8), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 0
    }, {
        .key = PIN(PORT_C, 9), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 0
    }, {
        .key = PIN(PORT_D, 2), /* TIM3_ETR */
        .channel = CHANNEL_ETR(2),
        .value = 0
    }, {
        .key = PIN(PORT_E, 2), /* TIM3_ETR */
        .channel = CHANNEL_ETR(2),
        .value = 0
    }, {
        .key = PIN(PORT_E, 3), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 0
    }, {
        .key = PIN(PORT_E, 4), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 0
    }, {
        .key = PIN(PORT_E, 5), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 0
    }, {
        .key = PIN(PORT_E, 6), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM14
    {
        .key = PIN(PORT_A, 4), /* TIM14_CH1 */
        .channel = CHANNEL_CH1(13),
        .value = 4
    }, {
        .key = PIN(PORT_A, 7), /* TIM14_CH1 */
        .channel = CHANNEL_CH1(13),
        .value = 4
    }, {
        .key = PIN(PORT_B, 1), /* TIM14_CH1 */
        .channel = CHANNEL_CH1(13),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM15
    {
        .key = PIN(PORT_A, 1), /* TIM15_CH1N */
        .channel = CHANNEL_CH1N(14),
        .value = 5
    }, {
        .key = PIN(PORT_A, 2), /* TIM15_CH1 */
        .channel = CHANNEL_CH1(14),
        .value = 0
    }, {
        .key = PIN(PORT_A, 3), /* TIM15_CH2 */
        .channel = CHANNEL_CH2(14),
        .value = 0
    }, {
        .key = PIN(PORT_A, 9), /* TIM15_BKIN */
        .channel = CHANNEL_BKIN(14),
        .value = 0
    }, {
        .key = PIN(PORT_B, 12), /* TIM15_BKIN */
        .channel = CHANNEL_BKIN(14),
        .value = 5
    }, {
        .key = PIN(PORT_B, 14), /* TIM15_CH1 */
        .channel = CHANNEL_CH1(14),
        .value = 1
    }, {
        .key = PIN(PORT_B, 15), /* TIM15_CH2 */
        .channel = CHANNEL_CH2(14),
        .value = 1
    }, {
        .key = PIN(PORT_B, 15), /* TIM15_CH1N */
        .channel = CHANNEL_CH1N(14),
        .value = 3
    }, {
        .key = PIN(PORT_F, 9), /* TIM15_CH1 */
        .channel = CHANNEL_CH1(14),
        .value = 0
    }, {
        .key = PIN(PORT_F, 10), /* TIM15_CH2 */
        .channel = CHANNEL_CH2(14),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM16
    {
        .key = PIN(PORT_A, 6), /* TIM16_CH1 */
        .channel = CHANNEL_CH1(15),
        .value = 5
    }, {
        .key = PIN(PORT_B, 5), /* TIM16_BKIN */
        .channel = CHANNEL_BKIN(15),
        .value = 2
    }, {
        .key = PIN(PORT_B, 6), /* TIM16_CH1N */
        .channel = CHANNEL_CH1N(15),
        .value = 2
    }, {
        .key = PIN(PORT_B, 8), /* TIM16_CH1 */
        .channel = CHANNEL_CH1(15),
        .value = 2
    }, {
        .key = PIN(PORT_E, 0), /* TIM16_CH1 */
        .channel = CHANNEL_CH1(15),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_TIM17
    {
        .key = PIN(PORT_A, 7), /* TIM17_CH1 */
        .channel = CHANNEL_CH1(16),
        .value = 5
    }, {
        .key = PIN(PORT_A, 10), /* TIM17_BKIN */
        .channel = CHANNEL_BKIN(16),
        .value = 0
    }, {
        .key = PIN(PORT_B, 4), /* TIM17_BKIN */
        .channel = CHANNEL_BKIN(16),
        .value = 5
    }, {
        .key = PIN(PORT_B, 7), /* TIM17_CH1N */
        .channel = CHANNEL_CH1N(16),
        .value = 2
    }, {
        .key = PIN(PORT_B, 9), /* TIM17_CH1 */
        .channel = CHANNEL_CH1(16),
        .value = 2
    }, {
        .key = PIN(PORT_E, 1), /* TIM17_CH1 */
        .channel = CHANNEL_CH1(16),
        .value = 0
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[17] = {NULL};
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
void TIM1_BRK_UP_TRG_COM_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM1
void TIM1_CC_ISR(void)
{
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
#ifdef CONFIG_PLATFORM_STM32_TIM14
void TIM14_ISR(void)
{
  instances[13]->handler(instances[13]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM15
void TIM15_ISR(void)
{
  instances[14]->handler(instances[14]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM16
void TIM16_ISR(void)
{
  instances[15]->handler(instances[15]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_TIM17
void TIM17_ISR(void)
{
  instances[16]->handler(instances[16]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock([[maybe_unused]] const struct GpTimerBase *timer)
{
  const uint32_t ahbClock = clockFrequency(MainClock);
  const uint32_t apbClock = clockFrequency(ApbClock);

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
