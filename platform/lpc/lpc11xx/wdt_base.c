/*
 * wdt_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/wdt_base.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *);

static enum Result wdtInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const WdtBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = wdtInit,
    .deinit = nullptr /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct WdtBase *instance = nullptr;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct WdtBase *object)
{
  if (instance == nullptr)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void WDT_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t wdtGetClock(const struct WdtBase *)
{
  return clockFrequency(WdtClock);
}
/*----------------------------------------------------------------------------*/
static enum Result wdtInit(void *object, const void *)
{
  struct WdtBase * const timer = object;

  if (setInstance(timer))
  {
    sysClockEnable(CLK_WDT);

    timer->handler = nullptr;
    timer->irq = WDT_IRQ;
    return E_OK;
  }
  else
    return E_BUSY;
}
