/*
 * gptimer_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gptimer_base.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
/* Pack capture or match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
struct TimerBlockDescriptor
{
  LPC_TIMER_Type *reg;
  /* Clock to register interface and to peripheral */
  enum sysClockBranch clock;
  /* Reset control identifier */
  enum sysDeviceReset reset;
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct GpTimerBase *,
    struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass tmrTable = {
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static const struct TimerBlockDescriptor timerBlockEntries[] = {
    {
        .reg = LPC_TIMER0,
        .clock = CLK_M4_TIMER0,
        .reset = RST_TIMER0
    },
    {
        .reg = LPC_TIMER1,
        .clock = CLK_M4_TIMER1,
        .reset = RST_TIMER1
    },
    {
        .reg = LPC_TIMER2,
        .clock = CLK_M4_TIMER2,
        .reset = RST_TIMER2
    },
    {
        .reg = LPC_TIMER3,
        .clock = CLK_M4_TIMER3,
        .reset = RST_TIMER3
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerCapturePins[] = {
    {
        .key = PIN(PORT_1, 12), /* T0_CAP1 */
        .channel = 0,
        .value = PACK_VALUE(4, 1)
    }, {
        .key = PIN(PORT_1, 13), /* T0_CAP0 */
        .channel = 0,
        .value = PACK_VALUE(4, 0)
    }, {
        .key = PIN(PORT_1, 17), /* T0_CAP3 */
        .channel = 0,
        .value = PACK_VALUE(4, 3)
    }, {
        .key = PIN(PORT_1, 20), /* T0_CAP2 */
        .channel = 0,
        .value = PACK_VALUE(4, 2)
    }, {
        .key = PIN(PORT_2, 0), /* T3_CAP0 */
        .channel = 3,
        .value = PACK_VALUE(6, 0)
    }, {
        .key = PIN(PORT_2, 1), /* T3_CAP1 */
        .channel = 3,
        .value = PACK_VALUE(6, 1)
    }, {
        .key = PIN(PORT_2, 2), /* T3_CAP2 */
        .channel = 3,
        .value = PACK_VALUE(6, 2)
    }, {
        .key = PIN(PORT_2, 6), /* T3_CAP3 */
        .channel = 3,
        .value = PACK_VALUE(6, 3)
    }, {
        .key = PIN(PORT_5, 0), /* T1_CAP0 */
        .channel = 1,
        .value = PACK_VALUE(5, 0)
    }, {
        .key = PIN(PORT_5, 1), /* T1_CAP1 */
        .channel = 1,
        .value = PACK_VALUE(5, 1)
    }, {
        .key = PIN(PORT_5, 2), /* T1_CAP2 */
        .channel = 1,
        .value = PACK_VALUE(5, 2)
    }, {
        .key = PIN(PORT_5, 3), /* T1_CAP3 */
        .channel = 1,
        .value = PACK_VALUE(5, 3)
    }, {
        .key = PIN(PORT_6, 1), /* T2_CAP0 */
        .channel = 2,
        .value = PACK_VALUE(5, 0)
    }, {
        .key = PIN(PORT_6, 2), /* T2_CAP1 */
        .channel = 2,
        .value = PACK_VALUE(5, 1)
    }, {
        .key = PIN(PORT_6, 3), /* T2_CAP2 */
        .channel = 2,
        .value = PACK_VALUE(5, 2)
    }, {
        .key = PIN(PORT_6, 6), /* T2_CAP3 */
        .channel = 2,
        .value = PACK_VALUE(5, 3)
    }, {
        .key = PIN(PORT_8, 4), /* T0_CAP0 */
        .channel = 0,
        .value = PACK_VALUE(7, 0)
    }, {
        .key = PIN(PORT_8, 5), /* T0_CAP1 */
        .channel = 0,
        .value = PACK_VALUE(7, 1)
    }, {
        .key = PIN(PORT_8, 6), /* T0_CAP2 */
        .channel = 0,
        .value = PACK_VALUE(7, 2)
    }, {
        .key = PIN(PORT_8, 7), /* T0_CAP3 */
        .channel = 0,
        .value = PACK_VALUE(7, 3)
    }, {
        .key = PIN(PORT_C, 1), /* T3_CAP0 */
        .channel = 3,
        .value = PACK_VALUE(6, 0)
    }, {
        .key = PIN(PORT_C, 4), /* T3_CAP1 */
        .channel = 3,
        .value = PACK_VALUE(6, 1)
    }, {
        .key = PIN(PORT_C, 5), /* T3_CAP2 */
        .channel = 3,
        .value = PACK_VALUE(6, 2)
    }, {
        .key = PIN(PORT_C, 6), /* T3_CAP3 */
        .channel = 3,
        .value = PACK_VALUE(6, 3)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerMatchPins[] = {
    {
        .key = PIN(PORT_1, 14), /* T0_MAT2 */
        .channel = 0,
        .value = PACK_VALUE(4, 2)
    }, {
        .key = PIN(PORT_1, 15), /* T0_MAT1 */
        .channel = 0,
        .value = PACK_VALUE(4, 1)
    }, {
        .key = PIN(PORT_1, 16), /* T0_MAT0 */
        .channel = 0,
        .value = PACK_VALUE(4, 0)
    }, {
        .key = PIN(PORT_1, 18), /* T0_MAT3 */
        .channel = 0,
        .value = PACK_VALUE(4, 3)
    }, {
        .key = PIN(PORT_2, 3), /* T3_MAT0 */
        .channel = 3,
        .value = PACK_VALUE(6, 0)
    }, {
        .key = PIN(PORT_2, 4), /* T3_MAT1 */
        .channel = 3,
        .value = PACK_VALUE(6, 1)
    }, {
        .key = PIN(PORT_2, 5), /* T3_MAT2 */
        .channel = 3,
        .value = PACK_VALUE(6, 2)
    }, {
        .key = PIN(PORT_2, 7), /* T3_MAT3 */
        .channel = 3,
        .value = PACK_VALUE(6, 3)
    }, {
        .key = PIN(PORT_5, 4), /* T1_MAT0 */
        .channel = 1,
        .value = PACK_VALUE(5, 0)
    }, {
        .key = PIN(PORT_5, 5), /* T1_MAT1 */
        .channel = 1,
        .value = PACK_VALUE(5, 1)
    }, {
        .key = PIN(PORT_5, 6), /* T1_MAT2 */
        .channel = 1,
        .value = PACK_VALUE(5, 2)
    }, {
        .key = PIN(PORT_5, 7), /* T1_MAT3 */
        .channel = 1,
        .value = PACK_VALUE(5, 3)
    }, {
        .key = PIN(PORT_6, 7), /* T2_MAT0 */
        .channel = 2,
        .value = PACK_VALUE(5, 0)
    }, {
        .key = PIN(PORT_6, 8), /* T2_MAT1 */
        .channel = 2,
        .value = PACK_VALUE(5, 1)
    }, {
        .key = PIN(PORT_6, 9), /* T2_MAT2 */
        .channel = 2,
        .value = PACK_VALUE(5, 2)
    }, {
        .key = PIN(PORT_6, 11), /* T2_MAT3 */
        .channel = 2,
        .value = PACK_VALUE(5, 3)
    }, {
        .key = PIN(PORT_8, 0), /* T0_MAT0 */
        .channel = 0,
        .value = PACK_VALUE(7, 0)
    }, {
        .key = PIN(PORT_8, 1), /* T0_MAT1 */
        .channel = 0,
        .value = PACK_VALUE(7, 1)
    }, {
        .key = PIN(PORT_8, 2), /* T0_MAT2 */
        .channel = 0,
        .value = PACK_VALUE(7, 2)
    }, {
        .key = PIN(PORT_8, 3), /* T0_MAT3 */
        .channel = 0,
        .value = PACK_VALUE(7, 3)
    }, {
        .key = PIN(PORT_C, 7), /* T3_MAT0 */
        .channel = 3,
        .value = PACK_VALUE(6, 0)
    }, {
        .key = PIN(PORT_C, 8), /* T3_MAT1 */
        .channel = 3,
        .value = PACK_VALUE(6, 1)
    }, {
        .key = PIN(PORT_C, 9), /* T3_MAT2 */
        .channel = 3,
        .value = PACK_VALUE(6, 2)
    }, {
        .key = PIN(PORT_C, 10), /* T3_MAT3 */
        .channel = 3,
        .value = PACK_VALUE(6, 3)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &tmrTable;
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
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void TIMER1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void TIMER2_ISR(void)
{
  descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void TIMER3_ISR(void)
{
  descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer
    __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;
  enum result res;

  timer->channel = config->channel;
  timer->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(timer->channel, 0, timer)) != E_OK)
    return res;

  const struct TimerBlockDescriptor *entry = &timerBlockEntries[timer->channel];

  /* Enable clock to register interface and peripheral */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  timer->irq = TIMER0_IRQ + timer->channel;
  timer->resolution = 32;
  timer->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;

  sysClockDisable(timerBlockEntries[timer->channel].clock);
  setDescriptor(timer->channel, timer, 0);
}
