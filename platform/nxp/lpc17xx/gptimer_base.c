/*
 * gptimer_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gptimer_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
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
  return sysCoreClock / sysClockDivToValue(DEFAULT_DIV);
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
