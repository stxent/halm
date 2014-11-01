/*
 * gptimer_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gptimer_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/* Pack capture or match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
struct TimerBlockDescriptor
{
  LPC_TIMER_Type *reg;
  enum sysPowerDevice power;
  enum sysClockDevice clock;
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct GpTimerBase *,
    struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass timerTable = {
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor timerBlockEntries[4] = {
    {
        .reg = LPC_TIMER0,
        .power = PWR_TIM0,
        .clock = CLK_TIMER0
    },
    {
        .reg = LPC_TIMER1,
        .power = PWR_TIM1,
        .clock = CLK_TIMER1
    },
    {
        .reg = LPC_TIMER2,
        .power = PWR_TIM2,
        .clock = CLK_TIMER2
    },
    {
        .reg = LPC_TIMER3,
        .power = PWR_TIM3,
        .clock = CLK_TIMER3
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerCapturePins[] = {
    {
        .key = PIN(0, 4), /* TIMER2_CAP0 */
        .channel = 2,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(0, 5), /* TIMER2_CAP1 */
        .channel = 2,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(0, 23), /* TIMER3_CAP0 */
        .channel = 3,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(0, 24), /* TIMER3_CAP1 */
        .channel = 3,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 18), /* TIMER1_CAP0 */
        .channel = 1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 19), /* TIMER1_CAP1 */
        .channel = 1,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 26), /* TIMER0_CAP0 */
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(1, 27), /* TIMER0_CAP1 */
        .channel = 0,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerMatchPins[] = {
    {
        .key = PIN(0, 6), /* TIMER2_MAT0 */
        .channel = 2,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(0, 7), /* TIMER2_MAT1 */
        .channel = 2,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(0, 8), /* TIMER2_MAT2 */
        .channel = 2,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(0, 9), /* TIMER2_MAT3 */
        .channel = 2,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = PIN(0, 10), /* TIMER3_MAT0 */
        .channel = 3,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(0, 11), /* TIMER3_MAT1 */
        .channel = 3,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 22), /* TIMER1_MAT0 */
        .channel = 1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 25), /* TIMER1_MAT1 */
        .channel = 1,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 28), /* TIMER0_MAT0 */
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 29), /* TIMER0_MAT1 */
        .channel = 0,
        .value = PACK_VALUE(3, 1)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(3, 25), /* TIMER0_MAT0 */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(3, 26), /* TIMER0_MAT1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(4, 28), /* TIMER2_MAT0 */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(4, 29), /* TIMER2_MAT1 */
        .channel = 2,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &timerTable;
static struct GpTimerBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct GpTimerBase *state, struct GpTimerBase *timer)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      timer) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void TIMER0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void TIMER1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void TIMER2_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void TIMER3_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer
    __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;
  enum result res;

  /* Try to set peripheral descriptor */
  timer->channel = config->channel;
  if ((res = setDescriptor(timer->channel, 0, timer)) != E_OK)
    return res;

  const struct TimerBlockDescriptor *entry = &timerBlockEntries[timer->channel];

  sysPowerEnable(entry->power);
  sysClockControl(entry->clock, DEFAULT_DIV);

  timer->handler = 0;
  timer->irq = TIMER0_IRQ + timer->channel;
  timer->resolution = 32;
  timer->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;

  sysPowerDisable(timerBlockEntries[timer->channel].power);
  setDescriptor(timer->channel, timer, 0);
}
