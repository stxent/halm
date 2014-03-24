/*
 * gptimer_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
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
const struct GpioDescriptor gpTimerCapturePins[] = {
    {
        .key = PIN(0, 2), /* CT16B0_CAP0 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 0), /* CT32B1_CAP0 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 5), /* CT32B0_CAP0 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 8), /* CT16B1_CAP0 */
        .channel = TIMER_CT16B1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor gpTimerMatchPins[] = {
    {
        .key = PIN(0, 1), /* CT32B0_MAT2 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(0, 8), /* CT16B0_MAT0 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(0, 9), /* CT16B0_MAT1 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(0, 10), /* CT16B0_MAT2 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(0, 11), /* CT32B0_MAT3 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = PIN(1, 1), /* CT32B1_MAT0 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(1, 2), /* CT32B1_MAT1 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(1, 3), /* CT32B1_MAT2 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(1, 4), /* CT32B1_MAT3 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = PIN(1, 6), /* CT32B0_MAT0 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(1, 7), /* CT32B0_MAT1 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(1, 9), /* CT16B1_MAT0 */
        .channel = TIMER_CT16B1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(1, 10), /* CT16B1_MAT1 */
        .channel = TIMER_CT16B1,
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

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = timer;
  return E_OK;
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
  return sysCoreClock;
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
      sysClockEnable(CLK_CT16B0);
      device->reg = LPC_TIMER16B0;
      device->irq = TIMER16B0_IRQ;
      break;
    case 1:
      sysClockEnable(CLK_CT16B1);
      device->reg = LPC_TIMER16B1;
      device->irq = TIMER16B1_IRQ;
      break;
    case 2:
      sysClockEnable(CLK_CT32B0);
      device->reg = LPC_TIMER32B0;
      device->irq = TIMER32B0_IRQ;
      break;
    case 3:
      sysClockEnable(CLK_CT32B1);
      device->reg = LPC_TIMER32B1;
      device->irq = TIMER32B1_IRQ;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  const enum sysClockDevice timerClock[] = {
      CLK_CT16B0, CLK_CT16B1, CLK_CT32B0, CLK_CT32B1
  };
  struct GpTimerBase *device = object;

  sysClockDisable(timerClock[device->channel]);
  setDescriptor(device->channel, 0);
}
