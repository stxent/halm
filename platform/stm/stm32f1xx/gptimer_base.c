/*
 * gptimer_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/stm/gptimer_base.h>
#include <halm/platform/stm/gptimer_defs.h>
#include <halm/platform/stm/stm32f1xx/clocking.h>
#include <halm/platform/stm/stm32f1xx/pin_remap.h>
#include <halm/platform/stm/stm32f1xx/system.h>
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
};
/*----------------------------------------------------------------------------*/
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
    }, {
        .reg = STM_TIM2,
        .clock = CLK_TIM2,
        .reset = RST_TIM2,
        .irq = TIM2_IRQ
    }, {
        .reg = STM_TIM3,
        .clock = CLK_TIM3,
        .reset = RST_TIM3,
        .irq = TIM3_IRQ
    }, {
        .reg = STM_TIM4,
        .clock = CLK_TIM4,
        .reset = RST_TIM4,
        .irq = TIM4_IRQ
    }, {
        .reg = STM_TIM5,
        .clock = CLK_TIM5,
        .reset = RST_TIM5,
        .irq = TIM5_IRQ
    }, {
        .reg = STM_TIM6,
        .clock = CLK_TIM6,
        .reset = RST_TIM6,
        .irq = TIM6_IRQ /* Virtual IRQ */
    }, {
        .reg = STM_TIM7,
        .clock = CLK_TIM7,
        .reset = RST_TIM7,
        .irq = TIM7_IRQ
    }, {
        .reg = STM_TIM8,
        .clock = CLK_TIM8,
        .reset = RST_TIM8,
        .irq = TIM8_CC_IRQ
    }, {
        .reg = STM_TIM9,
        .clock = CLK_TIM9,
        .reset = RST_TIM9,
        .irq = -1
    }, {
        .reg = STM_TIM10,
        .clock = CLK_TIM10,
        .reset = RST_TIM10,
        .irq = -1
    }, {
        .reg = STM_TIM11,
        .clock = CLK_TIM11,
        .reset = RST_TIM11,
        .irq = -1
    }, {
        .reg = STM_TIM12,
        .clock = CLK_TIM12,
        .reset = RST_TIM12,
        .irq = TIM12_IRQ /* Virtual IRQ */
    }, {
        .reg = STM_TIM13,
        .clock = CLK_TIM13,
        .reset = RST_TIM13,
        .irq = TIM13_IRQ /* Virtual IRQ */
    }, {
        .reg = STM_TIM14,
        .clock = CLK_TIM14,
        .reset = RST_TIM14,
        .irq = TIM14_IRQ /* Virtual IRQ */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerCaptureComparePins[] = {
    {
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
        .key = PIN(PORT_A, 6), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = 0
    }, {
        .key = PIN(PORT_A, 7), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = 0
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
        .key = PIN(PORT_B, 0), /* TIM3_CH3 */
        .channel = CHANNEL_CH3(2),
        .value = 0
    }, {
        .key = PIN(PORT_B, 0), /* TIM1_CH2N */
        .channel = CHANNEL_CH2N(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_B, 1), /* TIM3_CH4 */
        .channel = CHANNEL_CH4(2),
        .value = 0
    }, {
        .key = PIN(PORT_B, 1), /* TIM1_CH3N */
        .channel = CHANNEL_CH3N(0),
        .value = PACK_REMAP(REMAP_TIM1_PARTIAL, 1)
    }, {
        .key = PIN(PORT_B, 3), /* TIM2_CH2 */
        .channel = CHANNEL_CH2(1),
        .value = PACK_REMAP(REMAP_TIM2_LOWER, 1)
    }, {
        .key = PIN(PORT_B, 4), /* TIM3_CH1 */
        .channel = CHANNEL_CH1(2),
        .value = PACK_REMAP(REMAP_TIM3_PARTIAL, 2)
    }, {
        .key = PIN(PORT_B, 5), /* TIM3_CH2 */
        .channel = CHANNEL_CH2(2),
        .value = PACK_REMAP(REMAP_TIM3_PARTIAL, 2)
    }, {
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
        .key = PIN(PORT_B, 10), /* TIM2_CH3 */
        .channel = CHANNEL_CH3(1),
        .value = PACK_REMAP(REMAP_TIM2_UPPER, 2)
    }, {
        .key = PIN(PORT_B, 11), /* TIM2_CH4 */
        .channel = CHANNEL_CH4(1),
        .value = PACK_REMAP(REMAP_TIM2_UPPER, 2)
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
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[14] = {0};
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
  instances[timer->channel] = 0;
}
#endif
