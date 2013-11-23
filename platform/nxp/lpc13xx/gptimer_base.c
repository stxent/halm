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
static enum result setDescriptor(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = 0,
    .setEnabled = 0,
    .setFrequency = 0,
    .setOverflow = 0,
    .value = 0
};
/*----------------------------------------------------------------------------*/
const struct TimerClass *GpTimerBase = &timerTable;
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

  /* Disable Timer/Counter clock */
  sysClockDisable(timerClock[device->channel]);

  /* Reset Timer descriptor */
  setDescriptor(device->channel, 0);
}
