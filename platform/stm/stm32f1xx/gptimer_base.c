/*
 * gptimer_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/stm/gptimer_base.h>
#include <halm/platform/stm/stm32f1xx/clocking.h>
#include <halm/platform/stm/stm32f1xx/system.h>
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
};
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t);
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM_GPTIMER_NO_DEINIT
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
    {
        .reg = STM_TIM1,
        .clock = CLK_TIM1,
        .reset = RST_TIM1,
        .irq = TIM1_CC_IRQ
    },
    {
        .reg = STM_TIM2,
        .clock = CLK_TIM2,
        .reset = RST_TIM2,
        .irq = TIM2_IRQ
    },
    {
        .reg = STM_TIM3,
        .clock = CLK_TIM3,
        .reset = RST_TIM3,
        .irq = TIM3_IRQ
    },
    {
        .reg = STM_TIM4,
        .clock = CLK_TIM4,
        .reset = RST_TIM4,
        .irq = TIM4_IRQ
    },
    {
        .reg = STM_TIM5,
        .clock = CLK_TIM5,
        .reset = RST_TIM5,
        .irq = TIM5_IRQ
    },
    {
        .reg = STM_TIM6,
        .clock = CLK_TIM6,
        .reset = RST_TIM6,
        .irq = TIM6_IRQ /* Virtual IRQ */
    },
    {
        .reg = STM_TIM7,
        .clock = CLK_TIM7,
        .reset = RST_TIM7,
        .irq = TIM7_IRQ
    },
    {
        .reg = STM_TIM8,
        .clock = CLK_TIM8,
        .reset = RST_TIM8,
        .irq = TIM8_CC_IRQ
    },
    {
        .reg = STM_TIM9,
        .clock = CLK_TIM9,
        .reset = RST_TIM9,
        .irq = -1
    },
    {
        .reg = STM_TIM10,
        .clock = CLK_TIM10,
        .reset = RST_TIM10,
        .irq = -1
    },
    {
        .reg = STM_TIM11,
        .clock = CLK_TIM11,
        .reset = RST_TIM11,
        .irq = -1
    },
    {
        .reg = STM_TIM12,
        .clock = CLK_TIM12,
        .reset = RST_TIM12,
        .irq = TIM12_IRQ /* Virtual IRQ */
    },
    {
        .reg = STM_TIM13,
        .clock = CLK_TIM13,
        .reset = RST_TIM13,
        .irq = TIM13_IRQ /* Virtual IRQ */
    },
    {
        .reg = STM_TIM14,
        .clock = CLK_TIM14,
        .reset = RST_TIM14,
        .irq = TIM14_IRQ /* Virtual IRQ */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerCaptureComparePins[] = {
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[14] = {0};
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t channel)
{
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct GpTimerBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (!instances[channel])
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM1_BRK_ISR(void)
//{
//  if (instances[0])
//    instances[0]->handler(instances[0]);
//}
/*----------------------------------------------------------------------------*/
void TIM1_CC_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM1_UP_ISR(void)
//{
//  if (instances[0])
//    instances[0]->handler(instances[0]);
//}
///*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM1_TRG_COM_ISR(void)
//{
//  if (instances[0])
//    instances[0]->handler(instances[0]);
//}
/*----------------------------------------------------------------------------*/
void TIM2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void TIM3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void TIM4_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void TIM5_ISR(void)
{
  instances[4]->handler(instances[4]);
}
/*----------------------------------------------------------------------------*/
/* Virtual handler */
void TIM6_ISR(void)
{
  if (instances[5])
    instances[5]->handler(instances[5]);
}
/*----------------------------------------------------------------------------*/
void TIM7_ISR(void)
{
  instances[6]->handler(instances[6]);
}
/*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM8_BRK_ISR(void)
//{
//  if (instances[7])
//    instances[7]->handler(instances[7]);
//}
/*----------------------------------------------------------------------------*/
void TIM8_CC_ISR(void)
{
  instances[7]->handler(instances[7]);
}
/*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM8_TRG_COM_ISR(void)
//{
//  if (instances[7])
//    instances[7]->handler(instances[7]);
//}
///*----------------------------------------------------------------------------*/
///* Virtual handler */
//void TIM8_UP_ISR(void)
//{
//  if (instances[7])
//    instances[7]->handler(instances[7]);
//}
/*----------------------------------------------------------------------------*/
/* Virtual handler */
void TIM12_ISR(void)
{
  if (instances[11])
    instances[11]->handler(instances[11]);
}
/*----------------------------------------------------------------------------*/
/* Virtual handler */
void TIM13_ISR(void)
{
  if (instances[12])
    instances[12]->handler(instances[12]);
}
/*----------------------------------------------------------------------------*/
/* Virtual handler */
void TIM14_ISR(void)
{
  if (instances[13])
    instances[13]->handler(instances[13]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer)
{
  const uint32_t ahbFrequency = clockFrequency(MainClock);
  uint32_t apbFrequency = 0;

  if (timer->channel == 0 || (timer->channel >= 7 && timer->channel <= 10))
    apbFrequency = clockFrequency(Apb2Clock);
  else
    apbFrequency = clockFrequency(Apb1Clock);

  return apbFrequency == ahbFrequency ? ahbFrequency : apbFrequency * 2;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;

  timer->channel = config->channel;
  timer->handler = 0;

  if (!setInstance(timer->channel, timer))
    return E_BUSY;

  const struct TimerBlockDescriptor * const entry =
      &timerBlockEntries[timer->channel];

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  timer->irq = entry->irq;
  timer->reg = entry->reg;
  timer->resolution = 16;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;

  sysClockDisable(timerBlockEntries[timer->channel].clock);
  resetInstance(timer->channel);
}
#endif
