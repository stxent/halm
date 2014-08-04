/*
 * gptimer_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gptimer_base.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
/* Pack capture or match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct GpTimerBase *);
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
const struct PinEntry gpTimerCapturePins[] = {
    {
        .key = PIN(0, 2), /* CT16B0_CAP0 */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 0), /* CT32B1_CAP0 */
        .channel = 3,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 5), /* CT32B0_CAP0 */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 8), /* CT16B1_CAP0 */
        .channel = 1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerMatchPins[] = {
    {
        .key = PIN(0, 1), /* CT32B0_MAT2 */
        .channel = 2,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(0, 8), /* CT16B0_MAT0 */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(0, 9), /* CT16B0_MAT1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(0, 10), /* CT16B0_MAT2 */
        .channel = 0,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(0, 11), /* CT32B0_MAT3 */
        .channel = 2,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = PIN(1, 1), /* CT32B1_MAT0 */
        .channel = 3,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 2), /* CT32B1_MAT1 */
        .channel = 3,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 3), /* CT32B1_MAT2 */
        .channel = 3,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(1, 4), /* CT32B1_MAT3 */
        .channel = 3,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(1, 6), /* CT32B0_MAT0 */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 7), /* CT32B0_MAT1 */
        .channel = 2,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(1, 9), /* CT16B1_MAT0 */
        .channel = 1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(1, 10), /* CT16B1_MAT1 */
        .channel = 1,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const enum sysClockDevice timerClockMap[] = {
    CLK_CT16B0, CLK_CT16B1, CLK_CT32B0, CLK_CT32B1
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &timerTable;
static struct GpTimerBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct GpTimerBase *timer)
{
  assert(channel < sizeof(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), 0,
      timer) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void TIMER16B0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void TIMER16B1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void TIMER32B0_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void TIMER32B1_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(struct GpTimerBase *timer __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configPtr)
{
  const struct GpTimerBaseConfig * const config = configPtr;
  struct GpTimerBase * const timer = object;
  enum result res;

  /* Try to set peripheral descriptor */
  timer->channel = config->channel;
  if ((res = setDescriptor(timer->channel, timer)) != E_OK)
    return res;

  timer->handler = 0;

  sysClockEnable(timerClockMap[timer->channel]);

  switch (timer->channel)
  {
    case 0: /* CT16B0 */
      timer->reg = LPC_TIMER16B0;
      timer->irq = TIMER16B0_IRQ;
      timer->resolution = 16;
      break;

    case 1: /* CT16B1 */
      timer->reg = LPC_TIMER16B1;
      timer->irq = TIMER16B1_IRQ;
      timer->resolution = 16;
      break;

    case 2: /* CT32B0 */
      timer->reg = LPC_TIMER32B0;
      timer->irq = TIMER32B0_IRQ;
      timer->resolution = 32;
      break;

    case 3: /* CT32B1 */
      timer->reg = LPC_TIMER32B1;
      timer->irq = TIMER32B1_IRQ;
      timer->resolution = 32;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  struct GpTimerBase * const timer = object;

  sysClockDisable(timerClockMap[timer->channel]);
  setDescriptor(timer->channel, 0);
}
