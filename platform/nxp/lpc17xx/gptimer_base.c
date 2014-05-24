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
const struct GpioDescriptor gpTimerCapturePins[] = {
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
const struct GpioDescriptor gpTimerMatchPins[] = {
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
const struct EntityClass *GpTimerBase = &timerTable;
static struct GpTimerBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct GpTimerBase *timer)
{
  assert(channel < sizeof(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), 0,
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
uint32_t gpTimerGetClock(struct GpTimerBase *timer __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configPtr)
{
  const struct GpTimerBaseConfig * const config = configPtr;
  struct GpTimerBase *device = object;
  enum result res;

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if ((res = setDescriptor(device->channel, device)) != E_OK)
    return res;

  device->handler = 0;

  switch (device->channel)
  {
    case 0:
      sysPowerEnable(PWR_TIM0);
      sysClockControl(CLK_TIMER0, DEFAULT_DIV);
      device->reg = LPC_TIMER0;
      device->irq = TIMER0_IRQ;
      break;
    case 1:
      sysPowerEnable(PWR_TIM1);
      sysClockControl(CLK_TIMER1, DEFAULT_DIV);
      device->reg = LPC_TIMER1;
      device->irq = TIMER1_IRQ;
      break;
    case 2:
      sysPowerEnable(PWR_TIM2);
      sysClockControl(CLK_TIMER2, DEFAULT_DIV);
      device->reg = LPC_TIMER2;
      device->irq = TIMER2_IRQ;
      break;
    case 3:
      sysPowerEnable(PWR_TIM3);
      sysClockControl(CLK_TIMER3, DEFAULT_DIV);
      device->reg = LPC_TIMER3;
      device->irq = TIMER3_IRQ;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  const enum sysPowerDevice timerPower[] = {
      PWR_TIM0, PWR_TIM1, PWR_TIM2, PWR_TIM3
  };
  struct GpTimerBase *device = object;

  sysPowerDisable(timerPower[device->channel]);
  setDescriptor(device->channel, 0);
}
