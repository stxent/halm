/*
 * gptimer_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gptimer_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* Pack capture or match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
struct TimerBlockDescriptor
{
  LPC_TIMER_Type *reg;
  enum SysClockBranch clock;
  uint8_t flags;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
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
    {
        .reg = LPC_CT16B0,
        .clock = CLK_CT16B0,
        .flags = 0
    }, {
        .reg = LPC_CT16B1,
        .clock = CLK_CT16B1,
        .flags = 0
    }, {
        .reg = LPC_CT32B0,
        .clock = CLK_CT32B0,
        .flags = GPTIMER_FLAG_32_BIT
    }, {
        .reg = LPC_CT32B1,
        .clock = CLK_CT32B1,
        .flags = GPTIMER_FLAG_32_BIT
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerCapturePins[] = {
    {
        .key = PIN(0, 2), /* CT16B0_CAP0 */
        .channel = GPTIMER_CT16B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 0), /* CT32B1_CAP0 */
        .channel = GPTIMER_CT32B1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 5), /* CT32B0_CAP0 */
        .channel = GPTIMER_CT32B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 8), /* CT16B1_CAP0 */
        .channel = GPTIMER_CT16B1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerMatchPins[] = {
    {
        .key = PIN(0, 1), /* CT32B0_MAT2 */
        .channel = GPTIMER_CT32B0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(0, 8), /* CT16B0_MAT0 */
        .channel = GPTIMER_CT16B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(0, 9), /* CT16B0_MAT1 */
        .channel = GPTIMER_CT16B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(0, 10), /* CT16B0_MAT2 */
        .channel = GPTIMER_CT16B0,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(0, 11), /* CT32B0_MAT3 */
        .channel = GPTIMER_CT32B0,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = PIN(1, 1), /* CT32B1_MAT0 */
        .channel = GPTIMER_CT32B1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 2), /* CT32B1_MAT1 */
        .channel = GPTIMER_CT32B1,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 3), /* CT32B1_MAT2 */
        .channel = GPTIMER_CT32B1,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(1, 4), /* CT32B1_MAT3 */
        .channel = GPTIMER_CT32B1,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(1, 6), /* CT32B0_MAT0 */
        .channel = GPTIMER_CT32B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 7), /* CT32B0_MAT1 */
        .channel = GPTIMER_CT32B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(1, 9), /* CT16B1_MAT0 */
        .channel = GPTIMER_CT16B1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(1, 10), /* CT16B1_MAT1 */
        .channel = GPTIMER_CT16B1,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[4] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct GpTimerBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void CT16B0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CT16B1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void CT32B0_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void CT32B1_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock([[maybe_unused]] const struct GpTimerBase *timer)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;

  if (!setInstance(config->channel, timer))
    return E_BUSY;

  const struct TimerBlockDescriptor * const entry =
      &timerBlockEntries[config->channel];

  sysClockEnable(entry->clock);

  timer->channel = config->channel;
  timer->flags = entry->flags;
  timer->handler = NULL;
  timer->irq = CT16B0_IRQ + config->channel;
  timer->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;

  sysClockDisable(timerBlockEntries[timer->channel].clock);
  instances[timer->channel] = NULL;
}
#endif
