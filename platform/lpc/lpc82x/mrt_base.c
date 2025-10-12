/*
 * mrt_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/mrt_base.h>
#include <halm/platform/lpc/mrt_defs.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct MrtBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_MRT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const MrtBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static struct MrtBase *instances[4] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct MrtBase *object)
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
void MRT_ISR(void)
{
  const uint32_t status = LPC_MRT->IRQ_FLAG;

  if (status & IRQ_FLAG_GFLAG(0))
    instances[0]->handler(instances[0]);

  if (status & IRQ_FLAG_GFLAG(1))
    instances[1]->handler(instances[1]);

  if (status & IRQ_FLAG_GFLAG(2))
    instances[2]->handler(instances[2]);

  if (status & IRQ_FLAG_GFLAG(3))
    instances[3]->handler(instances[3]);

  LPC_MRT->IRQ_FLAG = status;
}
/*----------------------------------------------------------------------------*/
uint32_t mrtGetClock(const struct MrtBase *)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct MrtBaseConfig * const config = configBase;
  struct MrtBase * const timer = object;

  assert(config->channel < ARRAY_SIZE(instances));

  if (!setInstance(config->channel, timer))
    return E_BUSY;

  timer->channel = config->channel;
  timer->handler = NULL;
  timer->reg = LPC_MRT;

  if (!sysClockStatus(CLK_MRT))
  {
    sysClockEnable(CLK_MRT);
    sysResetPulse(RST_MRT);

    /* Enable interrupts for all channels */
    irqSetPriority(MRT_IRQ, CONFIG_PLATFORM_LPC_MRT_PRIORITY);
    irqEnable(MRT_IRQ);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_MRT_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct MrtBase * const timer = object;
  bool active = false;

  instances[timer->channel] = NULL;

  for (size_t index = 0; index < ARRAY_SIZE(instances); ++index)
  {
    if (instances[index] != NULL)
    {
      active = true;
      break;
    }
  }

  if (!active)
  {
    irqDisable(MRT_IRQ);
    sysClockDisable(CLK_MRT);
  }
}
#endif
